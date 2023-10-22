#include "json_loader.h"
#include <fstream>
#include <iostream>

using namespace model;

namespace json_loader {

std::string FileToString(const std::filesystem::path& json_path) {
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

boost::json::value JsonStringToObjec(
    const std::string& source_string)  // mb smth instead of const string&? string_view?
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

    model::Game game;

    auto file_content = FileToString(json_path);
    auto jv = JsonStringToObjec(file_content);

    if (!jv.get_object().contains("defaultDogSpeed")) {
        //?
        game.SetDefaultDogSpeed(4.5);
    }
    auto speed = jv.get_object()["defaultDogSpeed"].as_double();
    game.SetDefaultDogSpeed(speed);
    LoadMaps(jv, game);

    return game;
}

}  // namespace json_loader
