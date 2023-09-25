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
        std::cout << req.method() << std::endl;
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

        std::cout << req.target() << std::endl;
        auto& my_map = game_.GetMaps()[0];
        boost::json::value jv = boost::json::value_from(my_map);
        std::string serialized_json = boost::json::serialize(jv);
        response.body() = serialized_json;

        auto target = req.target();
        if (boost::starts_with(target, "/api/v1/maps/"))
        {
            // вызываем метод для обработки запросов на маршруты
            // выдать карту конкретную
            // auto response = handleMapsRequest(req);
            // send(response);
        }
        else if (boost::starts_with(target, "/api/v1/maps"))
        {
            // выдать список карт
            // вызываем метод для обработки запросов типа /api/second/{id}
            // auto response = handleSecondRequest(req);
            // send(response);
        }
        else
        {
            // если начало запроса неизвестно, возвращаем ошибку 404
            // auto response = makeErrorResponse(http::status::not_found, "Unknown endpoint");
            // send(response);
        }
        return response;
    }
    model::Game& game_;
};

}  // namespace http_handler
