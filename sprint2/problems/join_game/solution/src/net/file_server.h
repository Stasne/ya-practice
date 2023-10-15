#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <filesystem>

namespace beast = boost::beast;
namespace http = beast::http;
namespace fs = std::filesystem;

using Response = http::response<http::string_body>;
using Request = http::request<http::string_body>;
namespace files {
class FileServer {
public:
    FileServer(const std::filesystem::path& root);
    template <typename Body, typename Allocator>
    auto FileRequestResponse(http::request<Body, http::basic_fields<Allocator>>&& req) const {
        auto http_version = req.version();
        auto keep_alive = req.keep_alive();
        auto status = http::status::ok;
        Response response(status, http_version);
        response.keep_alive(keep_alive);

        FillFileResponse(std::string(req.target()), response);
        return response;
    }

private:
    void FillFileResponse(const fs::path& file_path, http::response<http::string_body>& response) const;

private:
    const std::filesystem::path rootPath_;
};
}  //namespace files