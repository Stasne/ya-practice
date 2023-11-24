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
    auto     operator<=>(const BagSlot&) const = default;
};

using Bag = std::vector<BagSlot>;
struct PlayingUnit {
    spDog    dog;
    Bag      bag;
    uint32_t score{0};
    bool     operator==(const PlayingUnit& rhs) const {
        return bag == rhs.bag && dog->Id() == rhs.dog->Id() && score == rhs.score;
    }
};

class GameSession {
public:
    GameSession(SessionConfiguration&& config, collision_detector::CollisionPrameters&& collisionParams,
                std::optional<uint32_t> id = std::nullopt);
    GameSession(const GameSession&) = delete;
    GameSession(GameSession&&)      = delete;
    std::string_view  GetName() const noexcept { return name_; }
    uint32_t          GetId() const { return id_; }
    const model::Map& GetMap() const { return map_; }

    void AddDog(const spDog doge);
    void DogAction(uint32_t dogId, DogDirection action);

    using LootPositions = std::unordered_map<uint32_t, model::MapLoot>;
    const LootPositions& GetLoot() const { return lootPositions_; }

    using Players = std::unordered_map<uint32_t, PlayingUnit>;
    const Players& GetPlayers() const { return players_; }

    void UpdateState(uint32_t tick_ms);

    void RestorePlayerLoot(uint32_t id, Bag bag, uint32_t score) {
        if (!players_.count(id))
            throw std::runtime_error("Dog restore error: unknown id");
        players_.at(id).bag   = std::move(bag);
        players_.at(id).score = score;
    }
    void RestoreMapLoot(LootPositions lp) { lootPositions_ = std::move(lp); }

    bool operator==(const GameSession& rhs) const {
        bool baseParams = id_ == rhs.GetId() && map_ == rhs.GetMap() && name_ == rhs.GetName();
        if (!baseParams)
            return false;
        return players_ == rhs.players_ && speed_ == rhs.speed_ && map_ == rhs.map_ &&
               bagCapacity_ == rhs.bagCapacity_ && randomSpawn_ == rhs.randomSpawn_ &&
               lootPositions_ == rhs.lootPositions_ && colliderParams_ == rhs.colliderParams_ &&
               collider_ == rhs.collider_;
    }

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