#include "json_loader.h"
#include <fstream>
#include <iostream>

using namespace model;

namespace json_loader
{

model::Game LoadGame(const std::filesystem::path& json_path)
{
    std::ifstream file(json_path);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << json_path << std::endl;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string file_content = buffer.str();

    boost::json::error_code ec;
    boost::json::value jv = boost::json::parse(file_content, ec);
    if (ec)
    {
        std::cerr << "Failed to parse file: " << ec.message() << std::endl;
    }

    model::Game game;
    for (const auto& map : jv.get_object().at("maps").get_array())
    {
        game.AddMap(value_to<Map>(map));
    }

    return game;
}

}  // namespace json_loader
