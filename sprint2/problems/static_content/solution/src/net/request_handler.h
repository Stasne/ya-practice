#pragma once
#include <boost/algorithm/string/predicate.hpp>
#include <boost/json.hpp>
#include <string_view>
#include "http_server.h"
#include "model.h"
#include "router.h"
namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;

class RequestHandler {
public:
    explicit RequestHandler(model::Game& game) : game_{game} { SetupRoutes(); }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        router_.Route(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
    }

private:
    void SetupRoutes();

    void get_map_handler(const Router::Request& request, Router::Response& response) const;
    void get_maps_list_handler(const Router::Request& request, Router::Response& response) const;
    void get_file_handler(const Router::Request& request, Router::Response& response) const;

private:
    model::Game& game_;
    Router router_;
};

}  // namespace http_handler
