#pragma once
#include <magic_defs.h>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <filesystem>
#include "response_m.h"
namespace beast = boost::beast;
namespace http = beast::http;
namespace fs = std::filesystem;

using Request = http::request<http::string_body>;
namespace files {
class FileServer {
public:
    FileServer(const std::filesystem::path& root);
    template <typename Body, typename Allocator>
    auto FileRequestResponse(http::request<Body, http::basic_fields<Allocator>>&& req) const {
        fs::path full_path = rootPath_;
        full_path += std::string(req.target());
        if (fs::is_directory(full_path)) {
            // Проверяем, существует ли файл index.htm или index.html
            fs::path index_htm_path = full_path / "index.htm";
            fs::path index_html_path = full_path / "index.html";
            if (fs::exists(index_htm_path)) {
                full_path /= index_htm_path;
            } else if (fs::exists(index_html_path)) {
                full_path /= index_html_path;
            }
        }
        if (!fs::exists(full_path) || fs::is_directory(full_path)) {
            return http_handler::Response::MakeJSON(http::status::not_found, ErrorMessage::FILE_404,
                                                    http_handler::Response::ContentType::PLAIN_TEXT);
        }

        std::ifstream file(full_path.string(), std::ios::binary);
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        auto mime_type = FileMimeType(full_path);
        return http_handler::Response::Make(http::status::ok, content, mime_type);
    }

private:
    std::string FileMimeType(fs::path file) const;

private:
    const fs::path rootPath_;
};
}  //namespace files