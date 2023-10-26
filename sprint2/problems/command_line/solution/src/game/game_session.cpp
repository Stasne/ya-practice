#include "game_session.h"
#include <map.h>
namespace {
static uint32_t SessionId{0};
}
namespace model {

GameSession::GameSession(const Map& map, double speed, bool randomSpawn, std::string_view name)
    : id_(SessionId++),
      map_(map),
      speed_(speed),
      randomSpawn_(randomSpawn),
      name_(name.empty() ? map.GetName() + '_' + std::to_string(id_) : std::string(name)) {}

void GameSession::AddDog(const game::spDog doge) {
    dogs_.push_back(doge);
    auto mapSpawnPoint = GameSession::GetSpawnPoint(map_, randomSpawn_);
    dogs_.back()->SetPosition(mapSpawnPoint);
    auto roadsForPoint = map_.GetRoadsForPoint(mapSpawnPoint);
}

void GameSession::DogAction(uint32_t dogId, game::DogDirection action) {
    auto foundDog = std::find_if(dogs_.begin(), dogs_.end(), [dogId](auto& dog) { return dog->Id() == dogId; });
    if (foundDog == dogs_.end()) {
        return;
    }
    (*foundDog)->SetDirection(action, speed_);
}

void GameSession::UpdateState(uint32_t tick_ms) {
    for (auto& spdog : dogs_) {
        game::PlayerPoint estimatePosition = spdog->EstimatePosition(tick_ms);
        auto boundPoint = BoundDogMovementToMap(spdog->Position(), estimatePosition);
        spdog->SetPosition(boundPoint);
        if (boundPoint != estimatePosition)
            spdog->SetSpeed(0);
    }
};
game::PlayerPoint GameSession::FitPointToRoad(const game::PlayerPoint& point, const Road& road) {
    auto boundPoint = point;
    auto leftBot = road.GetLeftBotCorner();
    auto rightTop = road.GetRightTopCorner();
    boundPoint.x = bound(leftBot.x, rightTop.x, boundPoint.x);
    boundPoint.y = bound(leftBot.y, rightTop.y, boundPoint.y);
    return boundPoint;
}
game::PlayerPoint GameSession::GetSpawnPoint(const Map& map, bool isRandom) {
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

game::PlayerPoint GameSession::BoundDogMovementToMap(const game::PlayerPoint start,
                                                     const game::PlayerPoint& finish) const {
    auto possibleRoads = map_.GetRoadsForPoint(start);

    if (!possibleRoads.size()) {
        //dog not on road???
        boost::json::value json{{"map", map_.GetName()}, {"x", start.x}, {"y", start.y}};
        Logger::Log(json, "Dog not on road!");
        return start;
    }

    game::PlayerPoint tmpNextPoint = GameSession::FitPointToRoad(finish, possibleRoads.front());
    for (const auto& road : possibleRoads) {
        auto roadBoundPoint = GameSession::FitPointToRoad(finish, road);
        if (start.VectorLength(roadBoundPoint) > start.VectorLength(tmpNextPoint))
            tmpNextPoint = roadBoundPoint;
    }
    return tmpNextPoint;
}

}  // namespace model