#include "file_server.h"
#include <basic_entities.h>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
namespace fs = std::filesystem;
namespace {

static constexpr auto DefaultPage{"/index.html"};

std::unordered_map<std::string, std::string> extension_to_mime_type_ = {
    {".htm", "text/html"},       {".html", "text/html"},
    {".css", "text/css"},        {".txt", "text/plain"},
    {".js", "text/javascript"},  {".json", "application/json"},
    {".xml", "application/xml"}, {".png", "image/png"},
    {".jpg", "image/jpeg"},      {".jpe", "image/jpeg"},
    {".jpeg", "image/jpeg"},     {".gif", "image/gif"},
    {".bmp", "image/bmp"},       {".ico", "image/vnd.microsoft.icon"},
    {".tiff", "image/tiff"},     {".tif", "image/tiff"},
    {".svg", "image/svg+xml"},   {".svgz", "image/svg+xml"},
    {".mp3", "audio/mpeg"}};

}  // namespace

files::FileServer::FileServer(const fs::path& root) : rootPath_(root) {
    if (!fs::exists(rootPath_))
        throw std::runtime_error("Отсутствует каталог для файл-сервера");
}

void files::FileServer::FillFileResponse(const fs::path& file_path, http::response<http::string_body>& response) const {

    fs::path full_path = rootPath_;
    full_path += file_path;
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
        response.result(http::status::not_found);
        response.set(http::field::content_type, ContentType::TEXT_PLAIN);
        response.body() = "File Not found";
        return;
    }

    std::ifstream file(full_path.string(), std::ios::binary);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    auto mime_type_it = extension_to_mime_type_.find(full_path.extension().string());
    std::string mime_type =
        mime_type_it != extension_to_mime_type_.end() ? mime_type_it->second : "application/octet-stream";

    response.set(http::field::content_type, mime_type);
    response.body() = content;
}