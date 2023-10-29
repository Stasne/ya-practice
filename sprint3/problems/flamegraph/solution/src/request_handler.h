#pragma once
#include <boost/algorithm/string/predicate.hpp>
#include <boost/json.hpp>
#include <string_view>
#include "http_server.h"
#include "model.h"
namespace http_handler
{
using namespace std::literals;
namespace beast = boost::beast;
namespace http = beast::http;
struct ContentType
{
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
    constexpr static std::string_view APP_JSON = "application/json"sv;
    // При необходимости внутрь ContentType можно добавить и другие типы контента
};
class RequestHandler
{
public:
    explicit RequestHandler(model::Game& game) : game_{game} {}

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send)
    {
        auto response = makeResponse(req);
        send(response);
        // Обработать запрос request и отправить ответ, используя send
    }

private:
    template <typename Body, typename Allocator>
    http::response<http::string_body> makeResponse(const http::request<Body, http::basic_fields<Allocator>>& req) const
    {
        auto status = http::status::ok;
        auto http_version = req.version();
        auto keep_alive = req.keep_alive();
        auto content_type = ContentType::APP_JSON;
        http::response<http::string_body> response(status, http_version);
        response.set(http::field::content_type, content_type);
        response.keep_alive(keep_alive);

        auto target = req.target();
        if (boost::starts_with(target, "/api/v1/maps/"))
        {
            // вызываем метод для обработки запросов на маршруты
            // выдать карту конкретную
            std::string prefix = "/api/v1/maps/";  // smells? (!)
            auto pos = target.find(prefix);

            model::Map::Id mapId(std::string(target.substr(pos + prefix.length())));
            const auto* map = game_.FindMap(mapId);
            if (!map)
            {
                // Если нет карты - возвращаем 404
                status = http::status::not_found;
                boost::json::object errorBodyj;
                errorBodyj["code"] = "mapNotFound";
                errorBodyj["message"] = "Map not found";
                std::string serialized_json = boost::json::serialize(errorBodyj);
                response.body() = serialized_json;
            }
            else
            {
                boost::json::value jv = boost::json::value_from(*map);
                std::string serialized_json = boost::json::serialize(jv);
                response.body() = serialized_json;
            }
        }
        else if (boost::starts_with(target, "/api/v1/maps"))
        {
            auto& maps = game_.GetMaps();
            boost::json::array maplist;
            for (const auto& map : maps)
            {
                boost::json::object mapj;
                mapj["id"] = *map.GetId();
                mapj["name"] = map.GetName();
                maplist.push_back(mapj);
            }
            std::string serialized_json = boost::json::serialize(maplist);
            response.body() = serialized_json;
        }
        else
        {
            // если начало запроса неизвестно, возвращаем ошибку 400
            status = http::status::bad_request;
            boost::json::object errorBodyj;
            errorBodyj["code"] = "badRequest";
            errorBodyj["message"] = "Bad request";
            std::string serialized_json = boost::json::serialize(errorBodyj);
            response.body() = serialized_json;
        }
        response.result(status);
        return response;
    }

private:
    model::Game& game_;
};

}  // namespace http_handler
