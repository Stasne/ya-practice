#pragma once

#include <logger.h>
#include <timer.h>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>
namespace http = boost::beast::http;
using Response = http::response<http::string_body>;
// Класс созданный как тренировка псевдо-декоратора
// чтоб отделить логгирование от функционала реквест хэндлера
// мне показалось это чрезмерным/надуманным в данном случае
// остальное логгирование будет внутри сорцов

template <typename ReqHandler>
class LoggingRequestHandler {
public:
    explicit LoggingRequestHandler(ReqHandler& rh) : handler_{rh} {}

    template <typename Body, typename Allocator, typename Send>
    void operator()(const boost::asio::ip::tcp::socket& socket,
                    http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        // timestamp 1
        utils::Timer t;
        boost::json::value request_log{{"ip", socket.remote_endpoint().address().to_string()},
                                       {"URI", req.target().to_string()},
                                       {"method", req.method_string()}};
        BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, request_log) << "request received";
        boost::json::object jResponseObj;
        jResponseObj["method"] = req.method_string();

        auto loggingResponse = [&t, &send, &jResponseObj](auto&& response) {
            auto responseTime = t.Stop();
            std::string content_type{};
            if (!response[http::field::content_type].empty()) {
                content_type = response[http::field::content_type].to_string();
            } else {
                content_type = "No Content Type field present";
            }
            jResponseObj["response_time"] = responseTime;
            jResponseObj["code"] = response.result_int();
            jResponseObj["content_type"] = content_type;
            send(response);
        };
        handler_(std::forward<decltype(req)>(req), std::move(loggingResponse));
        BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, boost::json::value(jResponseObj))
                                << "response sent";
    }

private:
    ReqHandler& handler_;
};
