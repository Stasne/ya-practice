#pragma once
#include <boost/algorithm/string/predicate.hpp>
#include <boost/json.hpp>
#include <string_view>
#include "api_router.h"
#include "file_server.h"
#include "http_server.h"
#include "model.h"
namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;
using Response = http::response<http::string_body>;

class RequestHandler {
public:
    explicit RequestHandler(model::Game& game, files::FileServer& fileserver) : game_{game}, files_(fileserver) {
        SetupRoutes();
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    Response operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        if (boost::starts_with(req.target(), "/api"))  //api request
            return apiRouter_.Route(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
        else  // file request
            return files_.FileRequestResponse(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
    }

private:
    void SetupRoutes();

    void get_map_handler(const ApiRouter::Request& request, ApiRouter::Response& response) const;
    void get_maps_list_handler(const ApiRouter::Request& request, ApiRouter::Response& response) const;

private:
    model::Game& game_;
    files::FileServer& files_;
    ApiRouter apiRouter_;
};

}  // namespace http_handler
