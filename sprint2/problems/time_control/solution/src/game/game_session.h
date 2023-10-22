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
    uint32_t GetId() const { return id_; };

    void AddDog(const game::spDog doge) {
        dogs_.push_back(doge);
        dogs_.back()->SetPosition(map_.GetSpawnPoint());
    };

    void DogAction(uint32_t dogId, game::GameAction action) {
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

    void UpdateState(double tick_ms) {
        for (auto& spdog : dogs_) {
            auto& dog = *spdog;
            auto dogSpeed = dog.Speed();
            auto dogCurrentPosition = dog.Position();
            auto moveDirection = dog.Direction();
            game::PlayerPoint nextPosition{dogCurrentPosition.x + dogSpeed.hor * tick_ms / 1000,
                                           dogCurrentPosition.y + dogSpeed.vert * tick_ms / 1000};
            // if (moveDirection == game::actions::Left || moveDirection == game::actions::Right) {
            // horizontal
            auto possibleRoads = map_.GetRoadsForPoint(nextPosition);
            if (!possibleRoads.size())
                possibleRoads = map_.GetRoadsForPoint(dogCurrentPosition);

            assert(possibleRoads.size());  // ДОЛЖНЫ БЫТЬ

            if (possibleRoads.front().FitPointToRoadMade(nextPosition))
                dog.SetDirection(game::GameAction(std::string(game::actions::Stop)));
            dog.SetPosition(nextPosition);

            // } else {
            //     // vertical
            // }
        }
    };

private:
    uint32_t id_;
    Dogs dogs_;
    std::string name_;
    const Map& map_;
    double speed_;
};
using spGameSession = std::shared_ptr<GameSession>;
}  // namespace model