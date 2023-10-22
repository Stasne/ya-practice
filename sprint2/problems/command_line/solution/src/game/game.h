#pragma once
#include <map.h>

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "game_session.h"
#include "players.h"

using namespace game;
namespace model {

class Game {
public:
    // Maps workflow
    using Maps = std::vector<Map>;
    using GameSessions = std::vector<spGameSession>;
    void AddMap(Map map);

    const Maps& GetMaps() const noexcept { return maps_; }

    const Map* FindMap(const Map::Id& id) const noexcept {
        if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
            return &maps_.at(it->second);
        }
        return nullptr;
    }
    //Players
    Players& PlayersHandler() { return players_; }

    //Gaming sessions
    spGameSession StartGame(const Map& map, std::string_view name) {
        auto foundSession = std::find_if(sessions_.begin(), sessions_.end(), [&map](auto& spSession) {
            return spSession->GetMap().GetId() == map.GetId();
        });
        if (foundSession != sessions_.end())
            return *foundSession;

        double mapSpeed = defaultSpeed_;
        if (map.GetMapSpeed())
            mapSpeed = *map.GetMapSpeed();
        sessions_.emplace_back(std::make_shared<GameSession>(map, mapSpeed, randomSpawn_, name));
        return sessions_.back();  //Т.к. на работу с апи стоит мьютекс, то безопасно
    }
    void SetRandomSpawnEnabled(bool isEnabled) { randomSpawn_ = isEnabled; }
    const spGameSession FindGame(const Dog& dog) {
        for (const auto& session : sessions_) {
            for (const auto& pDog : session->GetPlayingDogs()) {
                if (pDog->Id() == dog.Id())
                    return session;
            }
        }
        return {};
    }
    void TickTime(uint32_t tick_ms) {
        for (auto& session : sessions_)
            session->UpdateState(tick_ms);
    }
    void SetDefaultDogSpeed(double speed) { defaultSpeed_ = speed; }
    void SetRandomSpawnPoint(bool isRandom = false) { randomSpawn_ = isRandom; }

private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;
    GameSessions sessions_;

    Maps maps_;
    MapIdToIndex map_id_to_index_;
    Players players_;
    double defaultSpeed_;
    bool randomSpawn_ = false;
};

}  // namespace model
