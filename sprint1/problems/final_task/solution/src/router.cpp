#include "router.h"

#include <boost/beast/core.hpp>
#include <boost/json.hpp>
#include <functional>
#include <iostream>
#include <string>
using namespace std::literals;
namespace http = boost::beast::http;

namespace http_handler {

void Router::AddRoute([[maybe_unused]] const std::string_view http_method, const std::string_view path,
                      RequestHandler handler) {
    // Ранее хранили как http_method | path, но пока нафиг не надо
    routes_[path.data()] = std::move(handler);
}

void Router::BadRequest(Response& response, ResponseHandler&& send) const {
    auto status = http::status::bad_request;
    boost::json::object errorBodyj;
    errorBodyj["code"] = "badRequest";
    errorBodyj["message"] = "Bad request";
    std::string serialized_json = boost::json::serialize(errorBodyj);
    response.body() = serialized_json;
    send(response);
}
}  // namespace http_handler