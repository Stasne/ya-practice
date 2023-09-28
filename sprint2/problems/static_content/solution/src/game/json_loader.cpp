#include "json_loader.h"
#include <fstream>
#include <iostream>

using namespace model;

namespace json_loader {

std::string ToString(const std::filesystem::path& json_path) {
    // .json extension check?
    std::ifstream file(json_path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << json_path << std::endl;
        throw std::runtime_error("Failed to open .json file");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

boost::json::value ToJson(const std::string& source_string)  // mb smth instead of const string&? string_view?
{
    boost::json::error_code ec;
    boost::json::value jv = boost::json::parse(source_string, ec);
    if (ec) {
        std::cerr << "Failed to parse file: " << ec.message() << std::endl;
        throw std::runtime_error("Failed to parse .json file");
    }
    return jv;
}

void LoadMaps(const boost::json::value& jv, model::Game& game) {
    for (const auto& map : jv.get_object().at("maps").get_array()) {
        game.AddMap(value_to<Map>(map));
    }
}

model::Game LoadGame(const std::filesystem::path& json_path) {

    auto file_content = ToString(json_path);
    auto jv = ToJson(file_content);

    model::Game game;
    LoadMaps(jv, game);

    return game;
}

}  // namespace json_loader
