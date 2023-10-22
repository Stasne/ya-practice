#include "request_handler.h"

#include <magic_defs.h>
#include <response_m.h>
#include <functional>

using namespace std;
using namespace std::placeholders;
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
        .SetProcessFunction(bind(&RequestHandler::get_players, this, _1, _2));

    apiRouter_.AddRoute(Endpoint::GAME_STATE)
        .SetNeedAuthorization(true)
        .SetAllowedMethods({http::verb::head, http::verb::get}, "Method not allowed"sv,
                           MiscMessage::ALLOWED_GET_HEAD_METHOD)
        .SetProcessFunction(bind(&RequestHandler::get_game_state, this, _1, _2));

    apiRouter_.AddRoute(Endpoint::PLAYER_ACTION)
        .SetNeedAuthorization(true)
        .SetContentType(Response::ContentType::TEXT_JSON, "Wrong content type"sv)
        .SetAllowedMethods({http::verb::post}, "Method not allowed"sv, MiscMessage::ALLOWED_POST_METHOD)
        .SetProcessFunction(bind(&RequestHandler::post_player_action, this, _1, _2));

    apiRouter_.AddRoute(Endpoint::EXTERNAL_TIME_TICK)
        .SetContentType(Response::ContentType::TEXT_JSON, "Wrong content type"sv)
        .SetAllowedMethods({http::verb::post}, "Method not allowed"sv, MiscMessage::ALLOWED_POST_METHOD)
        .SetProcessFunction(bind(&RequestHandler::port_external_time_tick, this, _1));
}

StringResponse RequestHandler::get_map_handler(const http_handler::Request&& request) const {

    auto status = http::status::ok;
    auto target = request.target();
    auto pos = request.target().find(Endpoint::MAP);

    model::Map::Id mapId(std::string(request.target().substr(pos + Endpoint::MAP.length())));
    const auto* map = game_.FindMap(mapId);
    if (!map) {
        // Если нет карты - возвращаем 404
        return Response::MakeJSON(http::status::not_found, ErrorCode::MAP_404, ErrorMessage::FILE_404);
    }

    boost::json::value jv = boost::json::value_from(*map);
    std::string serialized_json = boost::json::serialize(jv);

    auto content_type = std::string(http_handler::Response::ContentType::TEXT_JSON);
    return Response::Make(http::status::ok, serialized_json, content_type);
}

StringResponse RequestHandler::get_maps_list_handler(const http_handler::Request&& request) const {
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

StringResponse RequestHandler::post_join_game(const http_handler::Request&& request) const {
    auto content_type = std::string(http_handler::Response::ContentType::TEXT_JSON);

    boost::json::value val;
    try {
        val = boost::json::parse(request.body());
    } catch (...) {
        return Response::MakeBadRequestInvalidArgument("Json object parsing error");
    }

    if (!val.is_object())
        return Response::MakeBadRequestInvalidArgument("Bad json object"sv);

    boost::json::object obj = val.as_object();
    if (!obj.contains("mapId") || !obj.contains("userName"))
        return Response::MakeBadRequestInvalidArgument(
            "Bad json object. Make sure to have all required fields mentioned"sv);

    const std::string userName(obj["userName"].get_string());
    if (userName.empty())
        return Response::MakeBadRequestInvalidArgument("Empty username"sv);

    const auto mapId = model::Map::Id(std::string(obj["mapId"].get_string()));
    const auto* selectedMap = game_.FindMap(mapId);
    if (!selectedMap)
        return Response::MakeJSON(http::status::not_found, ErrorCode::MAP_404, "Selected map wasn't found"sv);

    auto token = security::token::CreateToken();
    auto newPlayer = game_.PlayersHandler().NewPlayer(userName, token);
    if (!newPlayer) {
        security::token::RemoveToken(*token);
        return Response::MakeBadRequestInvalidArgument("User exists"sv);
    }
    // create session with selected map?
    auto session = game_.StartGame(*selectedMap, "game");
    // join player(dog) to session
    session->AddDog(newPlayer->GetDog());

    // return token and player id
    boost::json::value joinResponse{{"authToken", **token.get()}, {"playerId", newPlayer->Id()}};

    return Response::Make(http::status::ok, boost::json::serialize(joinResponse));
}

StringResponse RequestHandler::get_players(const Token& token, const http_handler::Request&& request) const {
    boost::json::object jPlayers;
    auto players = game_.PlayersHandler().PlayersMap();
    for (const auto& playerPair : players) {
        jPlayers[to_string(playerPair.first)] = boost::json::object{{"name", playerPair.second->Name()}};
    }
    auto content_type = std::string(http_handler::Response::ContentType::TEXT_JSON);
    std::string serialized_json = boost::json::serialize({jPlayers});
    return Response::Make(http::status::ok, serialized_json, content_type);
}

StringResponse RequestHandler::get_game_state(const Token& token, const http_handler::Request&& request) const {
    // get player session
    auto wpPlayer = game_.PlayersHandler().PlayerByToken(token);
    if (wpPlayer.expired()) {
        //  Та хз, вроде не должно быть такого, проверка на существование ранее делалась (токена)
        assert(false);
    }
    auto session = game_.FindGame(*wpPlayer.lock()->GetDog());
    if (!session)
        return http_handler::Response::MakeErrorUnknownToken("No game session was found for u");

    boost::json::object jObject;
    // get all dogs from session
    auto dogs = session->GetPlayingDogs();
    for (const auto& dog : dogs) {
        // auto dogOwner = game_.PlayersHandler().PlayerByDog(*dog);
        boost::json::object jDog;
        std::vector<double> pos{dog->Position().x, dog->Position().y};
        boost::json::array jPos(pos.begin(), pos.end());
        jDog["pos"] = jPos;

        std::vector<double> speed{dog->Position().x, dog->Position().y};
        boost::json::array jSpeed(speed.begin(), speed.end());
        jDog["speed"] = jSpeed;

        jDog["dir"] = dog->Direction();

        jObject[to_string(dog->Id())] = jDog;
    }

    auto content_type = std::string(http_handler::Response::ContentType::TEXT_JSON);
    boost::json::object jFinal;
    jFinal["players"] = jObject;
    return Response::Make(http::status::ok, boost::json::serialize(boost::json::value(jFinal)), content_type);
}

StringResponse RequestHandler::post_player_action(const Token& token, const http_handler::Request&& request) const {

    boost::json::value val;
    try {
        val = boost::json::parse(request.body());
    } catch (...) {
        return Response::MakeBadRequestInvalidArgument("Json object parsing error");
    }

    if (!val.is_object())
        return Response::MakeBadRequestInvalidArgument("Bad json object"sv);

    boost::json::object obj = val.as_object();
    if (!obj.contains("move"))
        return Response::MakeBadRequestInvalidArgument(
            "Bad json object. Make sure to have all required fields mentioned"sv);
    auto move = std::string(obj["move"].get_string());
    if (!move.empty() && !game::Player::IsActionValid(move))
        return Response::MakeBadRequestInvalidArgument("Wrong move value");

    // get player session
    auto wpPlayer = game_.PlayersHandler().PlayerByToken(token);
    if (wpPlayer.expired()) {
        //  Та хз, вроде не должно быть такого, проверка на существование ранее делалась (токена)
        assert(false);
    }
    auto session = game_.FindGame(*wpPlayer.lock()->GetDog());
    if (!session)
        return http_handler::Response::MakeErrorUnknownToken("No game session was found for u");

    auto action = game::GameAction(move);
    session->DogAction(wpPlayer.lock()->GetDog()->Id(), action);
    auto content_type = std::string(http_handler::Response::ContentType::TEXT_JSON);
    return Response::Make(http::status::ok, "");
}
StringResponse RequestHandler::port_external_time_tick(const http_handler::Request&& request) const {
    boost::json::value val;
    try {
        val = boost::json::parse(request.body());
    } catch (...) {
        return Response::MakeBadRequestInvalidArgument("Json object parsing error");
    }

    if (!val.is_object())
        return Response::MakeBadRequestInvalidArgument("Bad json object"sv);

    boost::json::object obj = val.as_object();
    if (!obj.contains("timeDelta"))
        return Response::MakeBadRequestInvalidArgument(
            "Bad json object. Make sure to have all required fields mentioned"sv);

    // Ensure that "timeDelta" is an integer before getting its value
    if (!obj["timeDelta"].is_int64())
        return Response::MakeBadRequestInvalidArgument("Expected integer for 'timeDelta'"sv);

    auto timeDelta = obj["timeDelta"].get_int64();

    game_.TickTime(timeDelta);
    auto content_type = std::string(http_handler::Response::ContentType::TEXT_JSON);
    return Response::Make(http::status::ok, "{}");
}

}  // namespace http_handler
