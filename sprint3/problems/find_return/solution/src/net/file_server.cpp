#include "file_server.h"
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
namespace fs = std::filesystem;
namespace {

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

std::string files::FileServer::FileMimeType(fs::path full_path) const {
    auto        mime_type_it = extension_to_mime_type_.find(full_path.extension().string());
    std::string mime_type =
        mime_type_it != extension_to_mime_type_.end() ? mime_type_it->second : "application/octet-stream";

    return mime_type;
}