#include "api_router.h"

#include <functional>
#include <iostream>
#include <string>
using namespace std::literals;
namespace http = boost::beast::http;

namespace http_handler {

ApiRouter::RequestHandler& ApiRouter::AddRoute(const std::string_view path) {
    return apiRoutes_[std::string(path)];
}

}  // namespace http_handler