#pragma once
#include <map.h>

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "players.h"
using namespace game;
namespace model {

class GameSession {
public:
    // using Id = util::Tagged<std::string, GameSession>;
    GameSession(const Map& map) : map_(map) {}

    // const Id& GetId() const noexcept { return id_; }
    std::string_view Name() const noexcept { return name_; }
    void AddPlayer(const Player& player) { dogs_.push_back(player.Dog()); };

private:
    using Dogs = std::vector<game::spDog>;
    Dogs dogs_;
    std::string name_;
    const Map& map_;
};

class Game {
public:
    // Maps workflow
    using Maps = std::vector<Map>;
    using GameSessions = std::vector<GameSession>;
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
    const GameSession& StartGame(const Map& map) {
        sessions_.emplace_back(map);
        return sessions_.back();  //Т.к. на работу с апи стоит мьютекс, то безопасно
    }

private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;
    GameSessions sessions_;

    Maps maps_;
    MapIdToIndex map_id_to_index_;
    Players players_;
};

}  // namespace model
