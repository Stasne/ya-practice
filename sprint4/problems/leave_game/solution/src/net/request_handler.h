#pragma once
#include <game.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/json.hpp>
#include <mutex>
#include <string_view>
#include "file_server.h"
#include "http_server.h"
#include "uri_element.h"
namespace http_handler {

namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;

class RequestHandler {
public:
    using EndpointProcessor = UriElement;
    using ResponseHandler   = std::function<void(StringResponse&)>;
    explicit RequestHandler(model::Game& game, files::FileServer& fileserver, bool localMode = false)
        : game_{game}, files_(fileserver) {
        SetupRoutes(localMode);
    }

    RequestHandler(const RequestHandler&)            = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        std::string croppedTarget = req.target();
        auto        stop          = croppedTarget.find('?');
        if (stop != std::string::npos)
            croppedTarget = (croppedTarget.substr(0, stop));

        if (boost::starts_with(croppedTarget, Endpoint::MAPS))  //map request
        {
            return send(apiRoutes_.at(Endpoint::MAPS)(std::move(req)));
        }
        if (boost::starts_with(croppedTarget, Endpoint::API))  //api request
        {
            std::lock_guard<std::mutex> lk(m_);
            if (apiRoutes_.count(croppedTarget))
                return send(apiRoutes_.at(croppedTarget)(std::move(req)));
            else
                send(http_handler::Response::MakeJSON(http::status::bad_request, ErrorCode::BAD_REQUEST,
                                                      ErrorMessage::INVALID_ENDPOINT));
        } else  // file request
            send(files_.FileRequestResponse(std::forward<decltype(req)>(req)));
    }

private:
    void SetupRoutes(bool localMode);

    StringResponse GetMap(std::string_view target) const;
    StringResponse GetMapsList(std::string_view body) const;
    StringResponse PostJoinGame(std::string_view body) const;
    StringResponse GetPlayers(const Token& token, std::string_view body) const;
    StringResponse GetGameState(const Token& token, std::string_view body) const;
    StringResponse PostPlayerAction(const Token& token, std::string_view body) const;
    StringResponse PostExternalTick(std::string_view body) const;
    StringResponse GetHighScores(std::string_view body) const;

private:
    model::Game&                                            game_;
    files::FileServer&                                      files_;
    std::mutex                                              m_;
    std::unordered_map<std::string_view, EndpointProcessor> apiRoutes_;
};

}  // namespace http_handler
