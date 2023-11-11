#pragma once
#include <logger.h>
#include <optional>
#include <string>

#include <model/tagged.h>
#include "response_m.h"
#include "token_machine.h"
namespace security {
using Token = security::token::Token;
static inline std::optional<Token> ExtractTokenFromStringViewAndCheckIt(std::string_view body) {
    if (body.size() < 8)
        return {};
    auto tmpToken = Token(std::string(body.substr(7)));

    return tmpToken;  // copy elission?
}

namespace http = boost::beast::http;  // isnt it bad to use same it in every header?
template <typename Fn, typename Body, typename Allocator>
http_handler::StringResponse ExecuteAuthorized(const http::request<Body, http::basic_fields<Allocator>>& req,
                                               Fn&& action) {
    auto token = ExtractTokenFromStringViewAndCheckIt(req.base()[http::field::authorization]);

    if (!token || !token::IsTokenCorrect(*token))
        return http_handler::Response::MakeErrorInvalidToken();

    if (!token::IsTokenValid(*token))
        return http_handler::Response::MakeErrorUnknownToken();

    return action(*token, req.body());
}
}  // namespace security