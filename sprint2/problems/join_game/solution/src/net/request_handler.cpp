#include "request_handler.h"
#include <magic_defs.h>
#include <response_m.h>
#include <functional>
using namespace std::placeholders;
using namespace std;
using namespace std::literals;
namespace http_handler {

namespace methods {
static constexpr std::string_view GET{"GET"sv};
static constexpr std::string_view POST{"POST"sv};
}  // namespace methods

void RequestHandler::SetupRoutes() {
    apiRouter_.AddRoute(Endpoint::MAPS_LIST)
        .SetAllowedMethods({http::verb::head, http::verb::get}, "Method not allowed"sv,
                           MiscMessage::ALLOWED_GET_HEAD_METHOD)
        .SetProcessFunction(bind(&RequestHandler::get_maps_list_handler, this, _1));

    apiRouter_.AddRoute(Endpoint::MAP)
        .SetAllowedMethods({http::verb::head, http::verb::get}, "Method not allowed"sv,
                           MiscMessage::ALLOWED_GET_HEAD_METHOD)
        .SetProcessFunction(bind(&RequestHandler::get_map_handler, this, _1));

    apiRouter_.AddRoute(Endpoint::JOIN_GAME)
        .SetContentType(Response::ContentType::TEXT_JSON, "Wrong content type"sv)
        .SetAllowedMethods({http::verb::post}, "Method not allowed"sv, MiscMessage::ALLOWED_POST_METHOD)
        .SetProcessFunction(bind(&RequestHandler::post_join_game, this, _1));

    apiRouter_.AddRoute(Endpoint::PLAYERS_LIST)
        .SetNeedAuthorization(true)
        .SetAllowedMethods({http::verb::head, http::verb::get}, "Method not allowed"sv,
                           MiscMessage::ALLOWED_GET_HEAD_METHOD)
        .SetProcessFunction(bind(&RequestHandler::get_players, this, _1));
}

StringResponse RequestHandler::get_map_handler(const ApiRouter::Request&& request) const {

    auto status = http::status::ok;
    auto target = request.target();
    auto pos = request.target().find(Endpoint::MAP);

    model::Map::Id mapId(std::string(request.target().substr(pos + Endpoint::MAP.length())));
    const auto* map = game_.FindMap(mapId);
    if (!map) {
        // Если нет карты - возвращаем 404
        return Response::MakeJSON(http::status::not_found, ErrorCode::FILE_404, ErrorMessage::FILE_404);
    }

    boost::json::value jv = boost::json::value_from(*map);
    std::string serialized_json = boost::json::serialize(jv);

    auto content_type = std::string(http_handler::Response::ContentType::TEXT_JSON);
    return Response::Make(http::status::ok, serialized_json, content_type);
}
StringResponse RequestHandler::get_maps_list_handler(const ApiRouter::Request&& request) const {
    auto content_type = std::string(http_handler::Response::ContentType::TEXT_JSON);
    auto& maps = game_.GetMaps();
    boost::json::array maplist;
    for (const auto& map : maps) {
        boost::json::object mapj;
        mapj["id"] = *map.GetId();
        mapj["name"] = map.GetName();
        maplist.push_back(mapj);
    }
    std::string serialized_json = boost::json::serialize(maplist);
    return Response::Make(http::status::ok, serialized_json, content_type);
}

StringResponse RequestHandler::post_join_game(const ApiRouter::Request&& request) const {
    auto content_type = std::string(http_handler::Response::ContentType::TEXT_JSON);
    return Response::Make(http::status::ok, "join game", content_type);
}

StringResponse RequestHandler::get_players(const ApiRouter::Request&& request) const {
    auto content_type = std::string(http_handler::Response::ContentType::TEXT_JSON);
    return Response::Make(http::status::ok, "serialized_json", content_type);
}
}  // namespace http_handler
