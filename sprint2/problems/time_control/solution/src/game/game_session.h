#pragma once
#include <dog.h>
#include <iostream>

namespace model {
namespace {
static uint32_t SessionId{0};
}
class GameSession {
public:
    // using Id = util::Tagged<std::string, GameSession>;
    GameSession(const Map& map, double speed, std::string_view name = "")
        : id_(SessionId++), map_(map), speed_(speed), name_(name) {}
    // const Id& GetId() const noexcept { return id_; }
    std::string_view Name() const noexcept { return name_; }
    uint32_t GetId() const { return id_; }
    const Map& GetMap() const { return map_; }
    void AddDog(const game::spDog doge) {
        dogs_.push_back(doge);
        auto mapSpawnPoint = map_.GetSpawnPoint();
        dogs_.back()->SetPosition(mapSpawnPoint);
        auto roadsForPoint = map_.GetRoadsForPoint(mapSpawnPoint);
        assert(roadsForPoint.size());  // DEBUG;
    };

    void DogAction(uint32_t dogId, game::DogDirection action) {
        auto foundDog = std::find_if(dogs_.begin(), dogs_.end(), [dogId](auto& dog) { return dog->Id() == dogId; });
        //find dog
        if (foundDog == dogs_.end()) {
            assert(false);
            return;
        }
        auto& dog = *foundDog;
        dog->SetDirection(action, speed_);
    }

    using Dogs = std::vector<game::spDog>;  //maybe unord_map?
    const Dogs& GetPlayingDogs() const { return dogs_; }

    void UpdateState(uint32_t tick_ms) {
        for (auto& spdog : dogs_) {
            game::PlayerPoint estimatePosition = spdog->EstimatePosition(tick_ms);
            auto boundPoint = BoundDogMovementToMap(spdog->Position(), estimatePosition);
            spdog->SetPosition(boundPoint);
            if (boundPoint != estimatePosition)
                spdog->SetSpeed(0);
        }
    };
    game::PlayerPoint BoundDogMovementToMap(const game::PlayerPoint start, const game::PlayerPoint& finish) {
        auto possibleRoads = map_.GetRoadsForPoint(start);

        if (!possibleRoads.size()) {
            //dog not on road???
            boost::json::value json{{"map", map_.GetName()}, {"x", start.x}, {"y", start.y}};
            BOOST_LOG_TRIVIAL(warning) << boost::log::add_value(additional_data, json) << "Dog not on road!";
            return start;
        }
        game::PlayerPoint tmpNextPoint = possibleRoads.front().FitPointToRoad(finish);
        for (const auto& road : possibleRoads) {
            auto roadBoundPoint = road.FitPointToRoad(finish);
            if (start.VectorLength(roadBoundPoint) > start.VectorLength(tmpNextPoint))
                tmpNextPoint = roadBoundPoint;
        }
        return tmpNextPoint;
    }

private:
    uint32_t id_;
    Dogs dogs_;
    std::string name_;
    const Map& map_;
    double speed_;
};
using spGameSession = std::shared_ptr<GameSession>;
}  // namespace model