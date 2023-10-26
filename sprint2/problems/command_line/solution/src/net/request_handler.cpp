#include "request_handler.h"
#include <magic_defs.h>
#include <response_m.h>
#include <serializer.h>
#include <functional>

using namespace std;
using namespace std::placeholders;
using namespace std::literals;
using jfields = utils::serialization::fields;
using namespace std::literals;
namespace http = boost::beast::http;

namespace http_handler {

std::string_view ExtractMapName(string_view target) {

    auto pos = target.find(Endpoint::MAP);
    if (pos != std::string_view::npos)
        return target.substr(pos + Endpoint::MAP.length());

    return "";
}

void RequestHandler::SetupRoutes(bool localMode) {

    apiRoutes_[Endpoint::MAPS]
        .SetAllowedMethods({http::verb::head, http::verb::get}, "Method not allowed"sv,
                           MiscMessage::ALLOWED_GET_HEAD_METHOD)
        .SetProcessFunction(bind(&RequestHandler::GetMap, this, _1));

    apiRoutes_[Endpoint::JOIN_GAME]
        .SetContentType(Response::ContentType::TEXT_JSON, "Wrong content type"sv)
        .SetAllowedMethods({http::verb::post}, "Method not allowed"sv, MiscMessage::ALLOWED_POST_METHOD)
        .SetProcessFunction(bind(&RequestHandler::PostJoinGame, this, _1));

    apiRoutes_[Endpoint::PLAYERS_LIST]
        .SetNeedAuthorization(true)
        .SetAllowedMethods({http::verb::head, http::verb::get}, "Method not allowed"sv,
                           MiscMessage::ALLOWED_GET_HEAD_METHOD)
        .SetProcessFunction(bind(&RequestHandler::GetPlayers, this, _1, _2));

    apiRoutes_[Endpoint::GAME_STATE]
        .SetNeedAuthorization(true)
        .SetAllowedMethods({http::verb::head, http::verb::get}, "Method not allowed"sv,
                           MiscMessage::ALLOWED_GET_HEAD_METHOD)
        .SetProcessFunction(bind(&RequestHandler::GetGameState, this, _1, _2));

    apiRoutes_[Endpoint::PLAYER_ACTION]
        .SetNeedAuthorization(true)
        .SetContentType(Response::ContentType::TEXT_JSON, "Wrong content type"sv)
        .SetAllowedMethods({http::verb::post}, "Method not allowed"sv, MiscMessage::ALLOWED_POST_METHOD)
        .SetProcessFunction(bind(&RequestHandler::PostPlayerAction, this, _1, _2));

    if (localMode)  // shitty?
        apiRoutes_[Endpoint::EXTERNAL_TIME_TICK]
            .SetContentType(Response::ContentType::TEXT_JSON, "Wrong content type"sv)
            .SetAllowedMethods({http::verb::post}, "Method not allowed"sv, MiscMessage::ALLOWED_POST_METHOD)
            .SetProcessFunction(bind(&RequestHandler::PostExternalTick, this, _1));
}

StringResponse RequestHandler::GetMap(std::string_view target) const {
    auto mapName = ExtractMapName(target);

    if (mapName.empty())
        return GetMapsList(target);

    model::Map::Id mapId{std::string(mapName)};

    const auto* map = game_.FindMap(mapId);
    if (!map)
        return Response::MakeJSON(http::status::not_found, ErrorCode::MAP_404, ErrorMessage::FILE_404);

    boost::json::value jv = boost::json::value_from(*map);
    std::string serialized_json = boost::json::serialize(jv);

    auto content_type = std::string(http_handler::Response::ContentType::TEXT_JSON);
    return Response::Make(http::status::ok, serialized_json, content_type);
}

StringResponse RequestHandler::GetMapsList(std::string_view body) const {
    auto content_type = std::string(http_handler::Response::ContentType::TEXT_JSON);
    auto& maps = game_.GetMaps();
    boost::json::array mapJsonArray;
    for (const auto& map : maps)
        mapJsonArray.push_back(utils::serialization::ToJsonObject(map));

    std::string serialized_json = boost::json::serialize(mapJsonArray);
    return Response::Make(http::status::ok, serialized_json, content_type);
}

StringResponse RequestHandler::PostJoinGame(std::string_view body) const {
    auto content_type = std::string(http_handler::Response::ContentType::TEXT_JSON);

    boost::json::value val;
    try {
        val = boost::json::parse(body);
    } catch (...) {
        return Response::MakeBadRequestInvalidArgument("Json object parsing error");
    }

    if (!val.is_object())
        return Response::MakeBadRequestInvalidArgument("Bad json object"sv);

    boost::json::object obj = val.as_object();
    if (!obj.contains(jfields::MAP_ID) || !obj.contains(jfields::USERNAME))
        return Response::MakeBadRequestInvalidArgument(
            "Bad json object. Make sure to have all required fields mentioned"sv);

    const std::string userName(obj[jfields::USERNAME].get_string());
    if (userName.empty())
        return Response::MakeBadRequestInvalidArgument(ErrorMessage::USERNAME_EMPTY);

    const auto mapId = model::Map::Id(std::string(obj[jfields::MAP_ID].get_string()));
    const auto* selectedMap = game_.FindMap(mapId);
    if (!selectedMap)
        return Response::MakeJSON(http::status::not_found, ErrorCode::MAP_404, "Selected map wasn't found"sv);

    auto token = security::token::CreateToken();
    auto newPlayer = game_.PlayersHandler().NewPlayer(userName, token);
    if (!newPlayer) {
        security::token::RemoveToken(*token);
        return Response::MakeBadRequestInvalidArgument(ErrorMessage::USERNAME_EXISTS);
    }

    auto session = game_.StartGame(*selectedMap);
    session->AddDog(newPlayer->GetDog());

    boost::json::value joinResponse{{jfields::AUTH_TOKEN, **token.get()}, {jfields::PLAYER_ID, newPlayer->Id()}};

    return Response::Make(http::status::ok, boost::json::serialize(joinResponse));
}

StringResponse RequestHandler::GetPlayers(const Token& token, std::string_view body) const {
    boost::json::object jPlayers;
    auto players = game_.PlayersHandler().PlayersMap();
    for (const auto& playerPair : players) {
        jPlayers[to_string(playerPair.first)] = boost::json::object{{jfields::NAME, playerPair.second->Name()}};
    }
    auto content_type = std::string(http_handler::Response::ContentType::TEXT_JSON);
    std::string serialized_json = boost::json::serialize({jPlayers});
    return Response::Make(http::status::ok, serialized_json, content_type);
}

StringResponse RequestHandler::GetGameState(const Token& token, std::string_view body) const {
    auto wpPlayer = game_.PlayersHandler().PlayerByToken(token);

    auto session = game_.FindGame(*wpPlayer.lock()->GetDog());
    if (!session)
        return http_handler::Response::MakeErrorUnknownToken("No game session was found for u");

    boost::json::object jObject;
    auto dogs = session->GetPlayingDogs();
    for (const auto& dog : dogs) {
        jObject[to_string(dog->Id())] = utils::serialization::ToJsonObject(*dog);
    }

    auto content_type = std::string(http_handler::Response::ContentType::TEXT_JSON);
    boost::json::object jFinal;
    jFinal["players"] = jObject;
    return Response::Make(http::status::ok, boost::json::serialize(boost::json::value(jFinal)), content_type);
}

StringResponse RequestHandler::PostPlayerAction(const Token& token, std::string_view body) const {
    boost::json::value val;
    try {
        val = boost::json::parse(body);
    } catch (...) {
        return Response::MakeBadRequestInvalidArgument("Json object parsing error");
    }

    if (!val.is_object())
        return Response::MakeBadRequestInvalidArgument("Bad json object"sv);

    boost::json::object obj = val.as_object();
    if (!obj.contains(jfields::MOVE))
        return Response::MakeBadRequestInvalidArgument(
            "Bad json object. Make sure to have all required fields mentioned"sv);
    auto move = std::string(obj[jfields::MOVE].get_string());
    if (!move.empty() && !game::Player::IsActionValid(move))
        return Response::MakeBadRequestInvalidArgument("Wrong move value");

    // get player session
    auto wpPlayer = game_.PlayersHandler().PlayerByToken(token);

    auto session = game_.FindGame(*wpPlayer.lock()->GetDog());
    if (!session)
        return http_handler::Response::MakeErrorUnknownToken("No game session was found for u");

    auto action = game::DogDirection(move);
    session->DogAction(wpPlayer.lock()->GetDog()->Id(), action);
    auto content_type = std::string(http_handler::Response::ContentType::TEXT_JSON);

    return Response::Make(http::status::ok, boost::json::serialize(boost::json::value(boost::json::object())));
}

StringResponse RequestHandler::PostExternalTick(std::string_view body) const {
    boost::json::value val;
    try {
        val = boost::json::parse(body);
    } catch (...) {
        return Response::MakeBadRequestInvalidArgument("Json object parsing error");
    }

    if (!val.is_object())
        return Response::MakeBadRequestInvalidArgument("Bad json object"sv);

    boost::json::object obj = val.as_object();
    if (!obj.contains(jfields::TIME_TICK_DELTA))
        return Response::MakeBadRequestInvalidArgument(
            "Bad json object. Make sure to have all required fields mentioned"sv);

    if (!obj[jfields::TIME_TICK_DELTA].is_int64())
        return Response::MakeBadRequestInvalidArgument("Expected integer for 'timeDelta'"sv);

    auto timeDelta = obj[jfields::TIME_TICK_DELTA].get_int64();

    game_.TickTime(timeDelta);
    auto content_type = std::string(http_handler::Response::ContentType::TEXT_JSON);
    return Response::Make(http::status::ok, boost::json::serialize(boost::json::value(boost::json::object())));
}

}  // namespace http_handler
