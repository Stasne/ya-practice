#pragma once
#include <model/dog.h>
#include <model/loot_generator.h>
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
    const uint32_t randomGeneratorPeriod;
    const double randomGeneratorProbability;
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

    using LootPositions = std::unordered_map<uint32_t, model::MapLoot>;
    const LootPositions& GetLoot() const { return lootPositions_; }
    void UpdateState(uint32_t tick_ms);

private:
    void UpdateDogsPosition(uint32_t tick_ms);
    void SpawnLoot(uint32_t tick_ms);

private:
    uint32_t id_;
    Dogs dogs_;
    std::string name_;
    const model::Map& map_;
    const double speed_;
    const bool randomSpawn_;
    loot_gen::LootGenerator lootGen_;
    LootPositions lootPositions_;
};

using spGameSession = std::shared_ptr<GameSession>;

}  // namespace game