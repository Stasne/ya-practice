#pragma once
#include <magic_defs.h>
#include <tagged.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <map>
#include "authorization.h"
#include "response_m.h"

namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;
// Class UriElement (Vladimir Mikhaylov(c)))
using Request = http::request<http::string_body>;
using FunctionWithAuthorize = std::function<StringResponse(const security::token::Token& token, Request&& req)>;
using FunctionWithoutAuthorize = std::function<StringResponse(Request&& req)>;
class UriElement {
    struct AllowedMethods {
        std::vector<http::verb> data_;
        std::string_view error_;
        std::string_view allowed_;

        AllowedMethods() : data_(), error_(), allowed_(){};
    };

    struct AuthorizeData {
        bool need_;
        AuthorizeData() : need_(false){};
    };

    struct ContentType {
        bool need_to_check_;
        std::string_view value_;
        std::string_view error_;

        ContentType() : need_to_check_(false){};
    };

public:
    UriElement() : methods_(), authorize_(), content_type_(){};
    UriElement& SetAllowedMethods(std::vector<http::verb> methods, std::string_view method_error_message,
                                  std::string_view allowed_methods) {
        methods_.data_ = std::move(methods);
        methods_.error_ = method_error_message;
        methods_.allowed_ = allowed_methods;

        return *this;
    }

    UriElement& SetNeedAuthorization(bool need_authorize_) {
        authorize_.need_ = need_authorize_;
        return *this;
    }

    UriElement& SetProcessFunction(FunctionWithAuthorize f) {
        process_function_ = std::move(f);
        return *this;
    }
    UriElement& SetProcessFunction(FunctionWithoutAuthorize f) {
        process_function_without_authorize_ = std::move(f);
        return *this;
    }
    UriElement& SetContentType(std::string_view type, std::string_view error_message) {
        content_type_.need_to_check_ = true;
        content_type_.value_ = type;
        content_type_.error_ = error_message;
        return *this;
    }

    template <typename Body, typename Allocator>
    StringResponse operator()(http::request<Body, http::basic_fields<Allocator>>&& req) {
        return ProcessRequest(std::move(req));
    }

    template <typename Body, typename Allocator>
    StringResponse ProcessRequest(http::request<Body, http::basic_fields<Allocator>>&& req) {
        if (methods_.data_.empty() ||
            std::find(methods_.data_.begin(), methods_.data_.end(), req.method()) != methods_.data_.end()) {
            //Check content type if necessary
            if (content_type_.need_to_check_ &&
                content_type_.value_ != std::string(req.base()[boost::beast::http::field::content_type])) {
                return http_handler::Response::MakeBadRequestInvalidArgument(content_type_.error_);
            }
            if (authorize_.need_) {
                return security::ExecuteAuthorized(req,
                                                   [&](const security::token::Token& token, std::string_view body) {
                                                       return process_function_(token, std::move(req));
                                                   });
            }

            auto stop = req.target().find('?');
            if (stop != std::string::npos) {
                return process_function_without_authorize_(std::move(req));
                // return process_function_without_authorize_(req.target().substr(stop + 1));
            }

            auto resp = process_function_without_authorize_(std::move(req));
            return resp;
        }
        return http_handler::Response::MakeMethodNotAllowed(methods_.error_, methods_.allowed_);
    }

private:
    AllowedMethods methods_;
    AuthorizeData authorize_;
    ContentType content_type_;
    FunctionWithAuthorize process_function_;
    FunctionWithoutAuthorize process_function_without_authorize_;
};
class ApiRouter {
public:
    // using StringResponse = http::response<http::string_body>;
    using RequestHandler = UriElement;
    using ResponseHandler = std::function<void(StringResponse&)>;

    template <typename Body, typename Allocator>
    auto Route(http::request<Body, http::basic_fields<Allocator>>&& req) {
        auto it = apiRoutes_.rbegin();
        for (it; it != apiRoutes_.rend(); ++it) {
            if (boost::starts_with(req.target(), it->first)) {
                return it->second(std::move(req));
            }
        }

        // if no such method exists
        return http_handler::Response::MakeJSON(http::status::bad_request, ErrorCode::BAD_REQUEST,
                                                ErrorMessage::INVALID_ENDPOINT);
    }

    RequestHandler& AddRoute(const std::string_view path);

private:
    void MakeBadRequest(StringResponse& response) const;

private:
    std::map<std::string, RequestHandler> apiRoutes_;
};

}  // namespace http_handler