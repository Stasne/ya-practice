#pragma once
#include <boost/algorithm/string/predicate.hpp>
#include <boost/json.hpp>
#include <mutex>
#include <string_view>
#include "api_router.h"
#include "file_server.h"
#include "game.h"
#include "http_server.h"
namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

class RequestHandler {
public:
    explicit RequestHandler(model::Game& game, files::FileServer& fileserver, bool localMode = false)
        : game_{game}, files_(fileserver) {
        SetupRoutes(localMode);
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        if (boost::starts_with(req.target(), Endpoint::API))  //api request
        {
            std::lock_guard<std::mutex> lk(m_);

            send(apiRouter_.Route(std::forward<decltype(req)>(req)));

        } else  // file request
            send(files_.FileRequestResponse(std::forward<decltype(req)>(req)));
    }

private:
    void SetupRoutes(bool localMode);

    StringResponse get_map_handler(std::string_view target) const;
    StringResponse get_maps_list_handler(std::string_view body) const;
    StringResponse post_join_game(std::string_view body) const;
    StringResponse get_players(const Token& token, std::string_view body) const;
    StringResponse get_game_state(const Token& token, std::string_view body) const;
    StringResponse post_player_action(const Token& token, std::string_view body) const;
    StringResponse port_external_time_tick(std::string_view body) const;

private:
    model::Game& game_;
    files::FileServer& files_;
    ApiRouter apiRouter_;
    std::mutex m_;
};

}  // namespace http_handler
