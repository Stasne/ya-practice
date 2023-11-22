#pragma once
#include <model/dog.h>
#include <model/map.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/json.hpp>
#include <string_view>
#include <vector>
using namespace std::literals;
namespace utils {
namespace serialization {
struct fields {
    static inline std::string_view ID{"id"sv};
    static inline std::string_view NAME{"name"sv};
    static inline std::string_view USERNAME{"userName"sv};
    static inline std::string_view PLAYER_ID{"playerId"sv};
    static inline std::string_view AUTH_TOKEN{"authToken"sv};
    static inline std::string_view MAP_ID{"mapId"sv};
    static inline std::string_view POSITION{"pos"sv};
    static inline std::string_view SPEED{"speed"sv};
    static inline std::string_view DIRECTION{"dir"sv};
    static inline std::string_view TIME_TICK_DELTA{"timeDelta"sv};
    static inline std::string_view MOVE{"move"sv};
    static inline std::string_view TYPE{"type"sv};
};

boost::json::object ToJsonObject(const game::Dog& dog) {
    using jf = utils::serialization::fields;
    boost::json::object jDog;
    std::vector<double> pos{dog.Position().x, dog.Position().y};
    boost::json::array  jPos(pos.begin(), pos.end());
    jDog[jf::POSITION] = jPos;

    std::vector<double> speed{dog.Speed().hor, dog.Speed().vert};
    boost::json::array  jSpeed(speed.begin(), speed.end());
    jDog[jf::SPEED] = jSpeed;

    jDog[jf::DIRECTION] = dog.Direction();
    return jDog;
}
boost::json::object ToJsonObject(const model::MapLoot& loot) {
    using jf = utils::serialization::fields;
    boost::json::object jLoot;
    std::vector<double> pos{loot.pos.x, loot.pos.y};
    boost::json::array  jPos(pos.begin(), pos.end());
    jLoot[jf::POSITION] = jPos;

    jLoot[jf::TYPE] = loot.type;

    return jLoot;
}
boost::json::object ToJsonObject(const model::Map& map) {
    using jf = utils::serialization::fields;
    boost::json::object mapj;
    mapj[jf::ID]   = *map.GetId();
    mapj[jf::NAME] = map.GetName();
    return mapj;
}
}  // namespace serialization
}  // namespace utils