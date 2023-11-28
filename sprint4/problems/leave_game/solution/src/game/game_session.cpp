#include "game_session.h"
#include <logger.h>
#include <model/map.h>
#include <random>
namespace {
static uint32_t SessionId{0};
}
using namespace model;
namespace game {

uint32_t GenerateRandomUint(size_t max) {
    if (max == 1)
        return 0;
    static uint32_t                                          seed{0};
    std::random_device                                       dev;
    std::mt19937                                             rng(dev() + seed++);
    std::uniform_int_distribution<std::mt19937::result_type> dist6(0, max - 1);
    return dist6(rng);
};

RealPoint BoundDogMovementToMap(const RealPoint start, const RealPoint& finish, const Map& map) {
    auto possibleRoads = map.GetRoadsForPoint(start);

    if (!possibleRoads.size()) {
        //dog not on road???
        boost::json::value json{{"map", map.GetName()}, {"x", start.x}, {"y", start.y}};
        Logger::Log(json, "Dog not on road!");
        return start;
    }

    RealPoint tmpNextPoint = possibleRoads.front().FitPointToRoad(finish);
    for (const auto& road : possibleRoads) {
        auto roadBoundPoint = road.FitPointToRoad(finish);
        if (start.VectorLength(roadBoundPoint) > start.VectorLength(tmpNextPoint))
            tmpNextPoint = roadBoundPoint;
    }
    return tmpNextPoint;
}

RealPoint GetNextMapPoint(const Map& map, bool isRandom = true) {
    auto& roads = map.GetRoads();
    if (!roads.size())
        return {0, 0};

    if (!isRandom)
        return {static_cast<double>(roads.front().GetStart().x), static_cast<double>(roads.front().GetStart().y)};

    const auto& chosenRoad = roads[GenerateRandomUint(roads.size())];
    uint32_t    lowestX    = std::min(chosenRoad.GetStart().x, chosenRoad.GetEnd().x);
    uint32_t    highestX   = std::max(chosenRoad.GetStart().x, chosenRoad.GetEnd().x);
    uint32_t    lowestY    = std::min(chosenRoad.GetStart().y, chosenRoad.GetEnd().y);
    uint32_t    highestY   = std::max(chosenRoad.GetStart().y, chosenRoad.GetEnd().y);

    uint32_t roadLength = std::max(highestX - lowestX, highestY - lowestY);
    auto     spawnX     = std::clamp(lowestX + GenerateRandomUint(roadLength), lowestX, highestX);
    auto     spawnY     = std::clamp(lowestY + GenerateRandomUint(roadLength), lowestY, highestY);
    return {static_cast<double>(spawnX), static_cast<double>(spawnY)};
}

GameSession::GameSession(SessionConfiguration&& config, collision_detector::CollisionPrameters&& collisionParams,
                         std::optional<uint32_t> id)
    : id_(id ? *id : SessionId++),
      name_(config.name.empty() ? config.map.GetName() + '_' + std::to_string(id_) : config.name),
      map_(config.map),
      speed_(config.speed),
      bagCapacity_(config.bagCapacity),
      randomSpawn_(config.randomSpawnPoint),
      lootGen_(std::chrono::seconds(config.randomGeneratorPeriod), config.randomGeneratorProbability),
      colliderParams_(std::move(collisionParams)),
      afkKickTimeout_(config.afkKickTimeout_ms) {
    uint32_t dummyOfficeId{0};
    for (const auto& mapOffice : map_.GetOffices()) {
        collider_.AddDropOffice({.ingame_id = dummyOfficeId++,
                                 .position  = geom::Point2D(mapOffice.GetPosition().x, mapOffice.GetPosition().y),
                                 .width     = colliderParams_.officeWidth});
    }
}

void GameSession::AddDog(const spDog doge) {
    players_[doge->Id()] = {
        .dog = doge, .bag = {}, .score = 0, .game_start = std::chrono::high_resolution_clock::now()};

    auto mapSpawnPoint = GetNextMapPoint(map_, randomSpawn_);
    doge->SetPosition(mapSpawnPoint);
    geom::Point2D start_pos{mapSpawnPoint.x, mapSpawnPoint.y};
    collider_.AddGatherer(
        {.ingame_id = doge->Id(), .start_pos = start_pos, .end_pos = start_pos, .width = colliderParams_.dogWidth});
}

void GameSession::RemoveDog(uint32_t dogId) {
    leftPlayers_[dogId] = players_[dogId];
    collider_.RemoveGatherer(dogId);
    // smells? YES, it is
    auto endTimepoint = std::chrono::high_resolution_clock::now();
    auto start =
        std::chrono::time_point_cast<GameTimePeriod>(leftPlayers_[dogId].game_start).time_since_epoch().count();
    auto end      = std::chrono::time_point_cast<GameTimePeriod>(endTimepoint).time_since_epoch().count();
    auto duration = end - start;
    leftPlayers_[dogId].play_time = GameTimePeriod(static_cast<uint32_t>(duration));
}

void GameSession::DogAction(uint32_t dogId, DogDirection action) {
    if (!players_.count(dogId) || !players_[dogId].dog)
        return;
    players_[dogId].dog->SetDirection(action, speed_);
}

void GameSession::UpdateState(GameTimePeriod tick_ms) {
    UpdateDogsPosition(tick_ms);
    UpdateGatherersPositions();
    ProcessCollisions();
    SpawnLoot(tick_ms);
    UpdateGatheringItems();
    CheckAfkPlayers(tick_ms);
}

void GameSession::UpdateDogsPosition(GameTimePeriod tick_ms) {
    for (auto& [_, player] : players_) {
        auto&     spdog            = player.dog;
        RealPoint estimatePosition = spdog->EstimatePosition(tick_ms.count());
        auto      boundPoint       = BoundDogMovementToMap(spdog->Position(), estimatePosition, map_);
        spdog->SetPosition(boundPoint);
        if (boundPoint != estimatePosition)
            spdog->SetSpeed(0);
    }
}

void GameSession::UpdateGatherersPositions() {
    // обновляет новую позицию собаки
    for (auto& [_, player] : players_) {
        auto& spdog = player.dog;
        collider_.UpdateNextTickPosition(spdog->Id(), {spdog->Position().x, spdog->Position().y});
    }
}

void GameSession::UpdateGatheringItems() {
    for (const auto& [id, item] : lootPositions_)
        collider_.AddItem({id, {item.pos.x, item.pos.y}, colliderParams_.itemWidth});
}

void GameSession::ProcessCollisions() {
    auto events = FindGatherEvents(collider_);
    for (const auto& e : events) {
        auto collisionType = e.type;
        switch (collisionType) {
            case collision_detector::CollisionEventType::ITEM_PICK: {
                if (!lootPositions_.count(e.item_id))  //item already picked
                    continue;

                auto& player = players_.at(e.gatherer_id);
                auto& item   = lootPositions_.at(e.item_id);
                if (player.bag.size() >= bagCapacity_)  // bag is full
                    continue;
                // add item to dogs' bag
                player.bag.push_back({e.item_id, item.type});
                // remove item from map
                lootPositions_.erase(e.item_id);
                break;
            }
            case collision_detector::CollisionEventType::ITEM_DROP: {
                auto& player = players_.at(e.gatherer_id);
                for (const auto& item : player.bag) {
                    player.score += map_.GetLootTypes()[item.type].value;
                }
                player.bag.clear();
                break;
            }
        }
    }
}

void GameSession::SpawnLoot(GameTimePeriod tick_ms) {
    auto lootToSpawn = lootGen_.Generate(tick_ms, lootPositions_.size(), players_.size());
    if (!lootToSpawn)
        return;
    static uint32_t lootNum;
    for (auto i = 0; i < lootToSpawn; ++i) {
        auto lootType  = GenerateRandomUint(map_.GetLootTypes().size());
        auto lootPoint = GetNextMapPoint(map_);
        lootPositions_.insert({lootNum++, {lootPoint, lootType}});
    }
}

void GameSession::CheckAfkPlayers(GameTimePeriod ticks) {
    for (auto& [id, playerCard] : players_) {
        if (playerCard.dog->IsActive()) {
            playerCard.afk_time = GameTimePeriod(0);
            continue;
        } else
            playerCard.afk_time += ticks;

        if (playerCard.afk_time >= afkKickTimeout_)  // player kick due to afk timeout
            RemoveDog(id);
    }

    for (auto& [id, playerCard] : leftPlayers_)
        players_.erase(id);
}
}  // namespace game
