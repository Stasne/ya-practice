#include "game_session.h"
#include <map.h>
#include <random>
namespace {
static uint32_t SessionId{0};
}
using namespace model;
namespace game {
uint32_t GenerateRandomUint(size_t max) {
    if (max == 1)
        return 0;
    std::random_device dev;
    std::mt19937 rng(dev());
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
    const auto& chosenRoad = roads[GenerateRandomUint(roads.size())];
    uint32_t lowestX = std::min(chosenRoad.GetStart().x, chosenRoad.GetEnd().x);
    uint32_t highestX = std::max(chosenRoad.GetStart().x, chosenRoad.GetEnd().x);
    uint32_t lowestY = std::min(chosenRoad.GetStart().y, chosenRoad.GetEnd().y);
    uint32_t highestY = std::max(chosenRoad.GetStart().y, chosenRoad.GetEnd().y);

    uint32_t roadLength = std::max(highestX - lowestX, highestY - lowestY);
    auto spawnX = std::clamp(lowestX + GenerateRandomUint(roadLength), lowestX, highestX);
    auto spawnY = std::clamp(lowestY + GenerateRandomUint(roadLength), lowestY, highestY);
    return {static_cast<double>(spawnX), static_cast<double>(spawnY)};
}

GameSession::GameSession(SessionConfiguration&& config)
    : id_(SessionId++),
      name_(config.name.empty() ? config.map.GetName() + '_' + std::to_string(id_) : config.name),
      map_(config.map),
      speed_(config.speed),
      randomSpawn_(config.randomSpawnPoint),
      lootGen_(std::chrono::seconds(config.randomGeneratorPeriod), config.randomGeneratorProbability) {}

void GameSession::AddDog(const spDog doge) {
    dogs_.push_back(doge);
    auto mapSpawnPoint = GetNextMapPoint(map_, randomSpawn_);
    dogs_.back()->SetPosition(mapSpawnPoint);
}

void GameSession::DogAction(uint32_t dogId, DogDirection action) {
    auto foundDog = std::find_if(dogs_.begin(), dogs_.end(), [dogId](auto& dog) { return dog->Id() == dogId; });
    if (foundDog == dogs_.end())
        return;

    (*foundDog)->SetDirection(action, speed_);
}

void GameSession::UpdateState(uint32_t tick_ms) {
    UpdateDogsPosition(tick_ms);
    //pick loot (if possible)
    //remove picked loot
    SpawnLoot(tick_ms);
}
void GameSession::UpdateDogsPosition(uint32_t tick_ms) {
    for (auto& spdog : dogs_) {
        RealPoint estimatePosition = spdog->EstimatePosition(tick_ms);
        auto boundPoint = BoundDogMovementToMap(spdog->Position(), estimatePosition, map_);
        spdog->SetPosition(boundPoint);
        if (boundPoint != estimatePosition)
            spdog->SetSpeed(0);
    }
}
void GameSession::SpawnLoot(uint32_t tick_ms) {
    auto lootToSpawn = lootGen_.Generate(std::chrono::milliseconds(tick_ms), lootPositions_.size(), dogs_.size());
    if (!lootToSpawn)
        return;
    static uint32_t lootNum;
    for (auto i = 0; i < lootToSpawn; ++i) {
        auto lootType = GenerateRandomUint(map_.GetLootTypes().size());
        lootPositions_.insert({lootNum++, {GetNextMapPoint(map_), lootType}});
    }
}

}  // namespace game