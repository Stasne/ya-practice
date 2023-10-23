#include "game.h"

#include <stdexcept>
using namespace std::literals;
namespace model {

void Game::AddMap(Map map) {
    const size_t index = maps_.size();
    if (auto [it, inserted] = map_id_to_index_.emplace(map.GetId(), index); !inserted) {
        throw std::invalid_argument("Map with id "s + *map.GetId() + " already exists"s);
    } else {
        try {
            maps_.emplace_back(std::move(map));
        } catch (...) {
            map_id_to_index_.erase(it);
            throw;
        }
    }
}

spGameSession Game::StartGame(const Map& map, std::string_view name) {
    auto foundSession = std::find_if(sessions_.begin(), sessions_.end(),
                                     [&map](auto& spSession) { return spSession->GetMap().GetId() == map.GetId(); });
    if (foundSession != sessions_.end())
        return *foundSession;

    double mapSpeed = defaultSpeed_;
    if (map.GetMapSpeed())
        mapSpeed = *map.GetMapSpeed();
    sessions_.emplace_back(std::make_shared<GameSession>(map, mapSpeed, randomSpawn_, name));
    return sessions_.back();  //Т.к. на работу с апи стоит мьютекс, то безопасно
}

}  // namespace model
