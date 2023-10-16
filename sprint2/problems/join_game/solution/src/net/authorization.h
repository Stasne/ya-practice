#pragma once

#include <optional>
#include <string>

#include <tagged.h>
#include "response_m.h"
#include "token_machine.h"
namespace security {
static inline std::optional<Token> ExtractTokenFromStringViewAndCheckIt(std::string_view body) {
    return {Token("12345678")};
}
template <typename Fn, typename Body, typename Allocator>
http_handler::StringResponse ExecuteAuthorized(
    const boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>>& req, Fn&& action) {
    if (auto token = ExtractTokenFromStringViewAndCheckIt(req.base()[boost::beast::http::field::authorization])) {
        return action(*token, req.body());
    } else {
        return http_handler::Response::MakeUnauthorizedErrorInvalidToken();
    }
}
}  // namespace security