#include "request_handler.h"
#include <functional>

using namespace std::placeholders;
using namespace std;
using namespace std::literals;
namespace http_handler {

namespace methods {
static constexpr std::string_view GET{"GET"sv};
static constexpr std::string_view POST{"POST"sv};
}  // namespace methods

namespace paths {
static constexpr std::string_view GetMap{"/api/v1/maps/"sv};
static constexpr std::string_view GetMapsList{"/api/v1/maps"sv};
}  // namespace paths

void RequestHandler::SetupRoutes() {
    apiRouter_.AddRoute(methods::GET, paths::GetMapsList, bind(&RequestHandler::get_maps_list_handler, this, _1, _2));
    apiRouter_.AddRoute(methods::GET, paths::GetMap, bind(&RequestHandler::get_map_handler, this, _1, _2));
}

void RequestHandler::get_map_handler(const ApiRouter::Request& request, ApiRouter::Response& response) const {

    auto status = http::status::ok;
    auto pos = request.target().find(std::string(paths::GetMap));

    model::Map::Id mapId(std::string(request.target().substr(pos + paths::GetMap.length())));
    const auto* map = game_.FindMap(mapId);
    if (!map) {
        // Если нет карты - возвращаем 404
        status = http::status::not_found;
        boost::json::object errorBodyj;
        errorBodyj["code"] = "mapNotFound";
        errorBodyj["message"] = "Map not found";
        std::string serialized_json = boost::json::serialize(errorBodyj);
        response.body() = serialized_json;
    } else {
        boost::json::value jv = boost::json::value_from(*map);
        std::string serialized_json = boost::json::serialize(jv);
        response.body() = serialized_json;
    }
    response.result(status);
}
void RequestHandler::get_maps_list_handler(const ApiRouter::Request& request, ApiRouter::Response& response) const {
    auto& maps = game_.GetMaps();
    boost::json::array maplist;
    for (const auto& map : maps) {
        boost::json::object mapj;
        mapj["id"] = *map.GetId();
        mapj["name"] = map.GetName();
        maplist.push_back(mapj);
    }

    std::string serialized_json = boost::json::serialize(maplist);
    response.body() = serialized_json;
}

}  // namespace http_handler
