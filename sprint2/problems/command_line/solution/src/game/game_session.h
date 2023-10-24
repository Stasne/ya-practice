#pragma once
#include <dog.h>
#include <iostream>

namespace model {
class Map;
class Road;
class GameSession {
public:
    GameSession(const Map& map, double speed, bool randomSpawn = false, std::string_view name = "");
    GameSession(const GameSession&) = delete;
    GameSession(GameSession&&) = delete;
    std::string_view GetName() const noexcept { return name_; }
    uint32_t GetId() const { return id_; }
    const Map& GetMap() const { return map_; }

    void AddDog(const game::spDog doge);
    void DogAction(uint32_t dogId, game::DogDirection action);

    using Dogs = std::vector<game::spDog>;
    const Dogs& GetPlayingDogs() const { return dogs_; }

    void UpdateState(uint32_t tick_ms);

private:
    static game::PlayerPoint FitPointToRoad(const game::PlayerPoint& point, const Road& road);
    static game::PlayerPoint GetSpawnPoint(const Map& map, bool isRandom);
    game::PlayerPoint BoundDogMovementToMap(const game::PlayerPoint start, const game::PlayerPoint& finish) const;

private:
    uint32_t id_;
    Dogs dogs_;
    std::string name_;
    const Map& map_;
    double speed_;
    const bool randomSpawn_;
};

using spGameSession = std::shared_ptr<GameSession>;

}  // namespace model