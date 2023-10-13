#pragma once

#include <logger.h>
#include <timer.h>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>
namespace http = boost::beast::http;
using Response = http::response<http::string_body>;

template <typename ReqHandler>
class LoggingRequestHandler {
public:
    explicit LoggingRequestHandler(ReqHandler& rh) : handler_{rh} {}

    template <typename Body, typename Allocator, typename Send>
    Response operator()(const boost::asio::ip::tcp::socket& socket,
                        http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        // timestamp 1
        utils::Timer t;
        boost::json::value request_log{{"ip", socket.remote_endpoint().address().to_string()},
                                       {"URI", req.target().to_string()},
                                       {"method", req.method_string()}};
        BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, request_log) << "request received";

        auto response = handler_(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
        // timestamp 2
        auto responseTime = t.Stop();
        std::string content_type = "null";
        auto it = response.find(http::field::content_type);
        if (it != response.end()) {
            content_type = it->value().to_string();
        }
        boost::json::value response_log{{"response_time", responseTime},
                                        {"code", response.result_int()},
                                        {"method", req.method_string()},
                                        {"content_type", content_type}};
        BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, response_log) << "response sent";
        return response;
    }

private:
    ReqHandler& handler_;
};
