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

spGameSession Game::StartGame(const Map& map, std::string_view name, std::optional<uint32_t> id) {
    auto foundSession = std::find_if(sessions_.begin(), sessions_.end(), [&map](auto& sessionIt) {
        const auto& spSession = sessionIt.second;
        return spSession->GetMap().GetId() == map.GetId();
    });
    if (foundSession != sessions_.end())
        return foundSession->second;

    double sessionSpeed = defaultSpeed_;
    if (map.GetMapSpeed())
        sessionSpeed = *map.GetMapSpeed();

    uint32_t bagCapacity = defaultBagCapacity_;
    if (map.GetBagCapacity())
        bagCapacity = *map.GetBagCapacity();

    game::SessionConfiguration             config{.name                       = std::string(name),
                                                  .map                        = map,
                                                  .speed                      = sessionSpeed,
                                                  .bagCapacity                = bagCapacity,
                                                  .randomSpawnPoint           = randomSpawn_,
                                                  .randomGeneratorPeriod      = randomGeneratorPeriod_,
                                                  .randomGeneratorProbability = randomGeneratorProbability_,
                                                  .afkKickTimeout_ms = static_cast<uint32_t>(afkKickTimeout_ * 1000)};
    collision_detector::CollisionPrameters collisionParams{
        .dogWidth = defaults::DOG_WIDTH, .officeWidth = defaults::OFFICE_WIDTH, .itemWidth = defaults::ITEM_WIDTH};
    auto session = std::make_shared<GameSession>(std::move(config), std::move(collisionParams), id);

    sessions_[session->GetId()] = session;
    return sessions_.at(session->GetId());  //Т.к. на работу с апи стоит мьютекс, то безопасно
}

}  // namespace model
