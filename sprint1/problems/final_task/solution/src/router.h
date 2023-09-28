#pragma once

#include <boost/algorithm/string/predicate.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/dynamic_body.hpp>

#include <map>

namespace http_handler {
namespace http = boost::beast::http;

struct ContentType {
    ContentType() = delete;  // TODO: why not simple namespace
    static constexpr std::string_view TEXT_HTML{"text/html"};
    static constexpr std::string_view APP_JSON{"application/json"};
    // При необходимости внутрь ContentType можно добавить и другие типы контента
};

class Router {
public:
    using Response = http::response<http::string_body>;
    using Request = http::request<http::string_body>;
    using RequestHandler = std::function<void(const Request&, Response&)>;
    using ResponseHandler = std::function<void(Response&)>;

    void AddRoute(const std::string& path, const RequestHandler& handler) { routes_[path] = handler; }

    template <typename Body, typename Allocator, typename Send>
    void Route(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) const {
        auto http_version = req.version();
        auto keep_alive = req.keep_alive();
        auto content_type = std::string(ContentType::APP_JSON);
        auto status = http::status::ok;

        Response response(status, http_version);

        response.set(http::field::content_type, content_type);
        response.keep_alive(keep_alive);

        auto it = routes_.rbegin();
        for (it; it != routes_.rend(); ++it) {
            if (boost::starts_with(req.target(), it->first)) {
                it->second(req, response);
                return send(response);
            }
        }

        // if no such method exists
        BadRequest(response, std::move(send));
    }

    void AddRoute([[maybe_unused]] const std::string_view http_method, const std::string_view path,
                  RequestHandler handler);

private:
    void BadRequest(Response& response, ResponseHandler&& send) const;

private:
    std::map<std::string, RequestHandler> routes_;
};

}  // namespace http_handler