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
    std::random_device                                       dev;
    std::mt19937                                             rng(dev());
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

    static uint32_t seed{0};
    const auto&     chosenRoad = roads[GenerateRandomUint(roads.size())];
    uint32_t        lowestX    = std::min(chosenRoad.GetStart().x, chosenRoad.GetEnd().x);
    uint32_t        highestX   = std::max(chosenRoad.GetStart().x, chosenRoad.GetEnd().x);
    uint32_t        lowestY    = std::min(chosenRoad.GetStart().y, chosenRoad.GetEnd().y);
    uint32_t        highestY   = std::max(chosenRoad.GetStart().y, chosenRoad.GetEnd().y);

    uint32_t roadLength = std::max(highestX - lowestX, highestY - lowestY);
    auto     spawnX     = std::clamp(lowestX + GenerateRandomUint(roadLength), lowestX, highestX);
    auto     spawnY     = std::clamp(lowestY + GenerateRandomUint(roadLength), lowestY, highestY);
    return {static_cast<double>(spawnX), static_cast<double>(spawnY)};
}

GameSession::GameSession(SessionConfiguration&& config, collision_detector::CollisionPrameters&& collisionParams)
    : id_(SessionId++),
      name_(config.name.empty() ? config.map.GetName() + '_' + std::to_string(id_) : config.name),
      map_(config.map),
      speed_(config.speed),
      bagCapacity_(config.bagCapacity),
      randomSpawn_(config.randomSpawnPoint),
      lootGen_(std::chrono::seconds(config.randomGeneratorPeriod), config.randomGeneratorProbability),
      colliderParams_(std::move(collisionParams)) {
    uint32_t dummyOfficeId{0};
    for (const auto& mapOffice : map_.GetOffices()) {
        collider_.AddDropOffice({.ingame_id = dummyOfficeId++,
                                 .position  = geom::Point2D(mapOffice.GetPosition().x, mapOffice.GetPosition().y),
                                 .width     = colliderParams_.officeWidth});
    }
}

void GameSession::AddDog(const spDog doge) {
    players_[doge->Id()] = {.dog = doge, .bag = {}, .score = 0};
    auto mapSpawnPoint   = GetNextMapPoint(map_, randomSpawn_);
    doge->SetPosition(mapSpawnPoint);
    geom::Point2D start_pos{mapSpawnPoint.x, mapSpawnPoint.y};
    collider_.AddGatherer(
        {.ingame_id = doge->Id(), .start_pos = start_pos, .end_pos = start_pos, .width = colliderParams_.dogWidth});
}

void GameSession::DogAction(uint32_t dogId, DogDirection action) {
    if (!players_.count(dogId) || !players_[dogId].dog)
        return;
    players_[dogId].dog->SetDirection(action, speed_);
}

void GameSession::UpdateState(uint32_t tick_ms) {
    UpdateDogsPosition(tick_ms);
    UpdateCollidableState();
    ProcessCollisions();
    SpawnLoot(tick_ms);
}
void GameSession::UpdateDogsPosition(uint32_t tick_ms) {
    for (auto& [_, player] : players_) {
        auto&     spdog            = player.dog;
        RealPoint estimatePosition = spdog->EstimatePosition(tick_ms);
        auto      boundPoint       = BoundDogMovementToMap(spdog->Position(), estimatePosition, map_);
        spdog->SetPosition(boundPoint);
        if (boundPoint != estimatePosition)
            spdog->SetSpeed(0);
    }
}
void GameSession::UpdateCollidableState() {
    // обновляет новую позицию собаки
    for (auto& [_, player] : players_) {
        auto& spdog = player.dog;
        collider_.UpdateNextTickPosition(spdog->Id(), {spdog->Position().x, spdog->Position().y});
    }
}
void GameSession::ProcessCollisions() {
    auto events = FindGatherEvents(collider_);
    for (const auto& e : events) {
        auto type = e.type;
        switch (type) {
            case collision_detector::CollisionEventType::ITEM_PICK: {
                if (!lootPositions_.count(e.item_id))  //item already picked
                    continue;

                auto& player = players_.at(e.gatherer_id);
                if (player.bag.size() >= bagCapacity_)  // bag is full
                    continue;
                // add item to dogs' bag
                player.bag.push_back({e.item_id, lootPositions_[e.item_id].type});
                // remove item from map
                lootPositions_.erase(e.item_id);
                break;
            }
            case collision_detector::CollisionEventType::ITEM_DROP: {
                auto& player = players_.at(e.gatherer_id);
                for (const auto& item : player.bag) {
                    // get item's value (from map config)
                    assert(item.item_id < map_.GetLootTypes().size());
                    // add scores
                    player.score += map_.GetLootTypes()[item.item_id].value;
                }
                player.bag.clear();
                break;
            }
        }
    }
}
void GameSession::SpawnLoot(uint32_t tick_ms) {
    auto lootToSpawn = lootGen_.Generate(std::chrono::milliseconds(tick_ms), lootPositions_.size(), players_.size());
    if (!lootToSpawn)
        return;
    static uint32_t lootNum;
    for (auto i = 0; i < lootToSpawn; ++i) {
        auto lootType = GenerateRandomUint(map_.GetLootTypes().size());
        lootPositions_.insert({lootNum++, {GetNextMapPoint(map_), lootType}});
    }
}

}  // namespace game