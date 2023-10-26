#include "game_session.h"
#include <map.h>
namespace {
static uint32_t SessionId{0};
}
using namespace model;
namespace game {

RealPoint GetSpawnPoint(const Map& map, bool isRandom) {
    auto& roads = map.GetRoads();
    if (!roads.size())
        return {0, 0};

    if (!isRandom)
        return {static_cast<double>(roads.front().GetStart().x), static_cast<double>(roads.front().GetStart().y)};

    static uint32_t seed{0};
    auto& chosenRoad = roads[seed++ % roads.size()];

    return {static_cast<double>(bound(chosenRoad.GetStart().x, chosenRoad.GetEnd().x, seed)),
            static_cast<double>(bound(chosenRoad.GetStart().y, chosenRoad.GetEnd().y, seed))};
}

GameSession::GameSession(SessionConfiguration&& config) : id_(SessionId++), config_(std::move(config)) {
    config_.name = config_.name.empty() ? config_.map.GetName() + '_' + std::to_string(id_) : std::string(config_.name);
}

void GameSession::AddDog(const spDog doge) {
    dogs_.push_back(doge);
    auto mapSpawnPoint = GetSpawnPoint(config_.map, config_.randomSpawnPoint);
    dogs_.back()->SetPosition(mapSpawnPoint);
    auto roadsForPoint = config_.map.GetRoadsForPoint(mapSpawnPoint);
}

void GameSession::DogAction(uint32_t dogId, DogDirection action) {
    auto foundDog = std::find_if(dogs_.begin(), dogs_.end(), [dogId](auto& dog) { return dog->Id() == dogId; });
    if (foundDog == dogs_.end()) {
        return;
    }
    (*foundDog)->SetDirection(action, config_.speed);
}

void GameSession::UpdateState(uint32_t tick_ms) {
    UpdateDogsPosition(tick_ms);
}
void GameSession::UpdateDogsPosition(uint32_t tick_ms) {
    for (auto& spdog : dogs_) {
        RealPoint estimatePosition = spdog->EstimatePosition(tick_ms);
        auto boundPoint = BoundDogMovementToMap(spdog->Position(), estimatePosition);
        spdog->SetPosition(boundPoint);
        if (boundPoint != estimatePosition)
            spdog->SetSpeed(0);
    }
}

RealPoint GameSession::BoundDogMovementToMap(const RealPoint start, const RealPoint& finish) const {
    auto possibleRoads = config_.map.GetRoadsForPoint(start);

    if (!possibleRoads.size()) {
        //dog not on road???
        boost::json::value json{{"map", config_.map.GetName()}, {"x", start.x}, {"y", start.y}};
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

}  // namespace game