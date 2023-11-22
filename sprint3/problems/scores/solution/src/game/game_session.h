#pragma once
#include <collisions/collision_detector.h>
#include <model/dog.h>
#include <model/loot_generator.h>

#include <iostream>
namespace model {
class Map;
class Road;
}  // namespace model

namespace game {

struct SessionConfiguration {
    std::string       name;
    const model::Map& map;
    const double      speed;
    const uint32_t    bagCapacity;
    const bool        randomSpawnPoint;
    const uint32_t    randomGeneratorPeriod;
    const double      randomGeneratorProbability;
};
struct BagSlot {
    uint32_t item_id;
    uint32_t type;
};
using Bag = std::vector<BagSlot>;
struct PlayingUnit {
    spDog    dog;
    Bag      bag;
    uint32_t score{0};
};
class GameSession {
public:
    GameSession(SessionConfiguration&& config, collision_detector::CollisionPrameters&& collisionParams);
    GameSession(const GameSession&) = delete;
    GameSession(GameSession&&)      = delete;
    std::string_view  GetName() const noexcept { return name_; }
    uint32_t          GetId() const { return id_; }
    const model::Map& GetMap() const { return map_; }

    void AddDog(const spDog doge);
    void AddPlayer(const spDog doge);
    void DogAction(uint32_t dogId, DogDirection action);

    using LootPositions = std::unordered_map<uint32_t, model::MapLoot>;
    const LootPositions& GetLoot() const { return lootPositions_; }
    void                 UpdateState(uint32_t tick_ms);
    using Players = std::unordered_map<uint32_t, PlayingUnit>;
    const Players& GetPlayers() const { return players_; }

private:
    void UpdateDogsPosition(uint32_t tick_ms);
    void SpawnLoot(uint32_t tick_ms);
    void UpdateCollidableState();
    void ProcessCollisions();

private:
    uint32_t                               id_;
    Players                                players_;
    std::string                            name_;
    const model::Map&                      map_;
    const double                           speed_;
    const uint32_t                         bagCapacity_;
    const bool                             randomSpawn_;
    loot_gen::LootGenerator                lootGen_;
    LootPositions                          lootPositions_;
    collision_detector::ItemsCollider      collider_;
    collision_detector::CollisionPrameters colliderParams_;
};

using spGameSession = std::shared_ptr<GameSession>;

}  // namespace game