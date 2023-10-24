#include "map.h"

using namespace std::literals;
namespace model {

namespace Fields {
constexpr static auto Id{"id"};
constexpr static auto Name{"name"};
constexpr static auto DogSpeed{"dogSpeed"};

constexpr static auto Buildings{"buildings"};
constexpr static auto Roads{"roads"};
constexpr static auto Offices{"offices"};

constexpr static auto X{"x"};
constexpr static auto Y{"y"};
constexpr static auto X0{"x0"};
constexpr static auto Y0{"y0"};
constexpr static auto X1{"x1"};
constexpr static auto Y1{"y1"};

constexpr static auto W{"w"};
constexpr static auto H{"h"};

constexpr static auto OffsetX{"offsetX"};
constexpr static auto OffsetY{"offsetY"};
}  // namespace Fields

Map::Roads Map::GetRoadsForPoint(const game::PlayerPoint& p) const noexcept {
    Roads result;
    for (const auto& road : roads_) {
        if (road.ContainsPoint(p)) {
            result.push_back(road);
        }
    }
    return result;
}

void Map::AddOffice(Office office) {
    if (warehouse_id_to_index_.contains(office.GetId())) {
        throw std::invalid_argument("Duplicate warehouse");
    }

    const size_t index = offices_.size();
    Office& o = offices_.emplace_back(std::move(office));
    try {
        warehouse_id_to_index_.emplace(o.GetId(), index);
    } catch (...) {
        // Удаляем офис из вектора, если не удалось вставить в unordered_map
        offices_.pop_back();
        throw;
    }
}

Road tag_invoke(boost::json::value_to_tag<Road>, const boost::json::value& jv) {
    if (!jv.is_object())
        throw std::runtime_error("Expected anboost::json::object");

    boost::json::object const& obj = jv.as_object();
    const auto x0 = obj.at(Fields::X0).as_int64();
    const auto y0 = obj.at(Fields::Y0).as_int64();
    const auto x1 =
        obj.if_contains(Fields::X1) ? std::optional<int>{value_to<int>(*obj.if_contains(Fields::X1))} : std::nullopt;
    const auto y1 =
        obj.if_contains(Fields::Y1) ? std::optional<int>{value_to<int>(*obj.if_contains(Fields::Y1))} : std::nullopt;
    if (x1) {
        return Road(model::Road::HORIZONTAL, model::Point(x0, y0), *x1);
    } else
        return Road(model::Road::VERTICAL, model::Point(x0, y0), *y1);
}

void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, const Road& road) {
    boost::json::object obj;
    obj[Fields::X0] = road.GetStart().x;
    obj[Fields::Y0] = road.GetStart().y;
    if (road.IsHorizontal())
        obj[Fields::X1] = road.GetEnd().x;
    else
        obj[Fields::Y1] = road.GetEnd().y;
    jv = std::move(obj);
}

void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, const Building& building) {
    boost::json::object obj;
    obj[Fields::X] = building.GetBounds().position.x;
    obj[Fields::Y] = building.GetBounds().position.y;
    obj[Fields::W] = building.GetBounds().size.width;
    obj[Fields::H] = building.GetBounds().size.height;
    jv = std::move(obj);
}

Building tag_invoke(boost::json::value_to_tag<Building>, const boost::json::value& jv) {
    if (!jv.is_object())
        throw std::runtime_error("Expected anboost::json::object");

    boost::json::object const& obj = jv.as_object();
    auto x = obj.at(Fields::X).as_int64();
    auto y = obj.at(Fields::Y).as_int64();
    auto w = obj.at(Fields::W).as_int64();
    auto h = obj.at(Fields::H).as_int64();
    Building building({model::Point(x, y), model::Size(w, h)});
    return building;
}

void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, const Office& office) {

    boost::json::object obj;
    obj[Fields::Id] = *office.GetId();
    obj[Fields::X] = office.GetPosition().x;
    obj[Fields::Y] = office.GetPosition().y;
    obj[Fields::OffsetX] = office.GetOffset().dx;
    obj[Fields::OffsetY] = office.GetOffset().dy;
    jv = std::move(obj);
}

Office tag_invoke(boost::json::value_to_tag<Office>, const boost::json::value& jv) {
    if (!jv.is_object())
        throw std::runtime_error("Expected anboost::json::object");

    boost::json::object const& obj = jv.as_object();
    Office::Id id(std::string(obj.at(Fields::Id).as_string()));
    auto x = obj.at(Fields::X).as_int64();
    auto y = obj.at(Fields::Y).as_int64();
    auto offsetX = obj.at(Fields::OffsetX).as_int64();
    auto offsetY = obj.at(Fields::OffsetY).as_int64();
    return Office(id, model::Point(x, y), model::Offset(offsetX, offsetY));
}

/// Map to json
void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, const Map& map) {
    boost::json::object obj;
    boost::json::array roads;
    boost::json::array buildings;
    boost::json::array offices;

    obj[Fields::Id] = *map.GetId();
    obj[Fields::Name] = map.GetName();

    for (const auto& road : map.GetRoads()) {
        roads.push_back(boost::json::value_from(road));
    }

    for (const auto& building : map.GetBuildings()) {
        buildings.push_back(boost::json::value_from(building));
    }

    for (const auto& office : map.GetOffices()) {
        offices.push_back(boost::json::value_from(office));
    }

    obj[Fields::Roads] = std::move(roads);
    obj[Fields::Buildings] = std::move(buildings);
    obj[Fields::Offices] = std::move(offices);

    jv = std::move(obj);
}

Map tag_invoke(boost::json::value_to_tag<Map>, const boost::json::value& jv) {
    if (!jv.is_object())
        throw std::runtime_error("Expected anboost::json::object");

    boost::json::object const& obj = jv.as_object();
    Map::Id id(std::string(obj.at(Fields::Id).as_string()));
    auto name = std::string(obj.at(Fields::Name).as_string());
    std::optional<double> dogSpeed;
    if (obj.contains(Fields::DogSpeed))
        dogSpeed = obj.at(Fields::DogSpeed).as_double();
    Map map(id, name, dogSpeed);
    for (const auto& road_value : obj.at(Fields::Roads).as_array()) {
        map.AddRoad(value_to<Road>(road_value));
    }

    for (const auto& building_value : obj.at(Fields::Buildings).as_array()) {
        map.AddBuilding(value_to<Building>(building_value));
    }

    for (const auto& office_value : obj.at(Fields::Offices).as_array()) {
        map.AddOffice(value_to<Office>(office_value));
    }

    return map;
}
}  // namespace model