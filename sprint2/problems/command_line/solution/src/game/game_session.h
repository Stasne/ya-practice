#pragma once
#include <dog.h>
#include <iostream>

namespace model {
class Map;
class Road;

}  // namespace model
namespace game {

struct SessionConfiguration {
    std::string name;
    const model::Map& map;
    const double speed;
    const bool randomSpawnPoint;
};

class GameSession {
public:
    GameSession(SessionConfiguration&& config);
    GameSession(const GameSession&) = delete;
    GameSession(GameSession&&) = delete;
    std::string_view GetName() const noexcept { return name_; }
    uint32_t GetId() const { return id_; }
    const model::Map& GetMap() const { return map_; }

    void AddDog(const spDog doge);
    void DogAction(uint32_t dogId, DogDirection action);

    using Dogs = std::vector<spDog>;
    const Dogs& GetPlayingDogs() const { return dogs_; }

    void UpdateState(uint32_t tick_ms);

private:
    void UpdateDogsPosition(uint32_t tick_ms);

private:
    uint32_t id_;
    Dogs dogs_;
    std::string name_;
    const model::Map& map_;
    const double speed_;
    const bool randomSpawn_;
};

using spGameSession = std::shared_ptr<GameSession>;

}  // namespace game