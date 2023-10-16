#pragma once
#include <boost/algorithm/string/predicate.hpp>
#include <boost/json.hpp>
#include <mutex>
#include <string_view>
#include "api_router.h"
#include "file_server.h"
#include "http_server.h"
#include "model.h"
namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
// using Strand = net::strand<net::io_context::executor_type>;
class RequestHandler {
public:
    explicit RequestHandler(model::Game& game, files::FileServer& fileserver)  //, Strand apiStrand)
        : game_{game}, files_(fileserver) {                                    //, apiStrand_(apiStrand) {
        SetupRoutes();
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        if (boost::starts_with(req.target(), Endpoint::API))  //api request
        {
            std::lock_guard<std::mutex> lk(m_);

            send(apiRouter_.Route(std::forward<decltype(req)>(req)));

            // using strand took too much time and led to fail...

            // auto handle = [self = shared_from_this(), send, req = std::forward<decltype(req)>(req)]() mutable {
            //     try {
            //         // Этот assert не выстрелит, так как лямбда-функция будет выполняться внутри strand
            //         assert(self->apiStrand_.running_in_this_thread());
            //         return send(std::move(self->apiRouter_.Route(std::move(req))));
            //     } catch (...) {
            //         // send(self->ReportServerError(version, keep_alive));
            //     }
            // };
            // // return net::dispatch(apiStrand_, handle);
            // return net::dispatch(apiStrand_, handle);

        } else  // file request
            send(files_.FileRequestResponse(std::forward<decltype(req)>(req)));
    }

private:
    void SetupRoutes();

    StringResponse get_map_handler(const ApiRouter::Request&& request) const;
    StringResponse get_maps_list_handler(const ApiRouter::Request&& request) const;
    StringResponse post_join_game(const ApiRouter::Request&& request) const;
    StringResponse get_players(const ApiRouter::Request&& request) const;

private:
    model::Game& game_;
    files::FileServer& files_;
    ApiRouter apiRouter_;
    // Strand& apiStrand_;
    std::mutex m_;
};

}  // namespace http_handler
