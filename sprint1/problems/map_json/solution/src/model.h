#pragma once
#include <boost/json.hpp>
#include <boost/json/value_to.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "tagged.h"

namespace model
{

using Dimension = int;
using Coord = Dimension;

struct Point
{
    Coord x, y;
};

struct Size
{
    Dimension width, height;
};

struct Rectangle
{
    Point position;
    Size size;
};

struct Offset
{
    Dimension dx, dy;
};

class Road
{
    struct HorizontalTag
    {
        explicit HorizontalTag() = default;
    };
    struct VerticalTag
    {
        explicit VerticalTag() = default;
    };

public:
    constexpr static HorizontalTag HORIZONTAL{};
    constexpr static VerticalTag VERTICAL{};

    Road(HorizontalTag, Point start, Coord end_x) noexcept : start_{start}, end_{end_x, start.y} {}

    Road(VerticalTag, Point start, Coord end_y) noexcept : start_{start}, end_{start.x, end_y} {}

    bool IsHorizontal() const noexcept { return start_.y == end_.y; }

    bool IsVertical() const noexcept { return start_.x == end_.x; }

    Point GetStart() const noexcept { return start_; }

    Point GetEnd() const noexcept { return end_; }
    friend void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, const Road& road)
    {
        boost::json::object obj;
        obj["x0"] = road.GetStart().x;
        obj["y0"] = road.GetStart().y;
        if (road.IsHorizontal())
            obj["x1"] = road.GetEnd().x;
        else
            obj["y1"] = road.GetEnd().y;
        jv = std::move(obj);
    }
    friend Road tag_invoke(boost::json::value_to_tag<Road>, const boost::json::value& jv)
    {
        if (!jv.is_object())
            throw std::runtime_error("Expected anboost::json::object");

        boost::json::object const& obj = jv.as_object();
        const auto x0 = obj.at("x0").as_int64();
        const auto y0 = obj.at("y0").as_int64();
        const auto x1 =
            obj.if_contains("x1") ? std::optional<int>{value_to<int>(*obj.if_contains("x1"))} : std::nullopt;
        const auto y1 =
            obj.if_contains("y1") ? std::optional<int>{value_to<int>(*obj.if_contains("y1"))} : std::nullopt;
        if (x1)
        {
            return Road(model::Road::HORIZONTAL, model::Point(x0, y0), *x1);
        }
        else
            return Road(model::Road::VERTICAL, model::Point(x0, y0), *y1);
    }

private:
    Point start_;
    Point end_;
};

class Building
{
public:
    explicit Building(Rectangle bounds) noexcept : bounds_{bounds} {}

    const Rectangle& GetBounds() const noexcept { return bounds_; }
    friend void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, const Building& building)
    {
        boost::json::object obj;
        obj["x"] = building.GetBounds().position.x;
        obj["y"] = building.GetBounds().position.y;
        obj["w"] = building.GetBounds().size.width;
        obj["h"] = building.GetBounds().size.height;
        jv = std::move(obj);
    }
    friend Building tag_invoke(boost::json::value_to_tag<Building>, const boost::json::value& jv)
    {
        if (!jv.is_object())
            throw std::runtime_error("Expected anboost::json::object");

        boost::json::object const& obj = jv.as_object();
        auto x = obj.at("x").as_int64();
        auto y = obj.at("y").as_int64();
        auto w = obj.at("w").as_int64();
        auto h = obj.at("h").as_int64();
        Building building({model::Point(x, y), model::Size(w, h)});
        return building;
    }

private:
    Rectangle bounds_;
};

class Office
{
public:
    using Id = util::Tagged<std::string, Office>;

    Office(Id id, Point position, Offset offset) noexcept : id_{std::move(id)}, position_{position}, offset_{offset} {}

    const Id& GetId() const noexcept { return id_; }

    Point GetPosition() const noexcept { return position_; }

    Offset GetOffset() const noexcept { return offset_; }
    friend void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, const Office& office)
    {

        boost::json::object obj;
        obj["id"] = *office.GetId();
        obj["x"] = office.GetPosition().x;
        obj["y"] = office.GetPosition().y;
        obj["offsetX"] = office.GetOffset().dx;
        obj["offsetY"] = office.GetOffset().dy;
        jv = std::move(obj);
    }
    friend Office tag_invoke(boost::json::value_to_tag<Office>, const boost::json::value& jv)
    {
        if (!jv.is_object())
            throw std::runtime_error("Expected anboost::json::object");

        boost::json::object const& obj = jv.as_object();
        Office::Id id(std::string(obj.at("id").as_string()));
        auto x = obj.at("x").as_int64();
        auto y = obj.at("y").as_int64();
        auto offsetX = obj.at("offsetX").as_int64();
        auto offsetY = obj.at("offsetY").as_int64();
        return Office(id, model::Point(x, y), model::Offset(offsetX, offsetY));
    }

private:
    Id id_;
    Point position_;
    Offset offset_;
};

class Map
{
public:
    using Id = util::Tagged<std::string, Map>;
    using Roads = std::vector<Road>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;

    Map(Id id, std::string name) noexcept : id_(std::move(id)), name_(std::move(name)) {}

    const Id& GetId() const noexcept { return id_; }

    const std::string& GetName() const noexcept { return name_; }

    const Buildings& GetBuildings() const noexcept { return buildings_; }

    const Roads& GetRoads() const noexcept { return roads_; }

    const Offices& GetOffices() const noexcept { return offices_; }

    void AddRoad(const Road& road) { roads_.emplace_back(road); }

    void AddBuilding(const Building& building) { buildings_.emplace_back(building); }

    void AddOffice(Office office);
    friend void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, const Map& map)
    {
        boost::json::object obj;
        boost::json::array roads;
        boost::json::array buildings;
        boost::json::array offices;

        obj["id"] = *map.GetId();
        obj["name"] = map.GetName();

        for (const auto& road : map.GetRoads())
        {
            roads.push_back(boost::json::value_from(road));
        }

        for (const auto& building : map.GetBuildings())
        {
            buildings.push_back(boost::json::value_from(building));
        }

        for (const auto& office : map.GetOffices())
        {
            offices.push_back(boost::json::value_from(office));
        }

        obj["roads"] = std::move(roads);
        obj["buildings"] = std::move(buildings);
        obj["offices"] = std::move(offices);

        jv = std::move(obj);
    }
    friend Map tag_invoke(boost::json::value_to_tag<Map>, const boost::json::value& jv)
    {
        if (!jv.is_object())
            throw std::runtime_error("Expected anboost::json::object");

        boost::json::object const& obj = jv.as_object();
        Map::Id id(std::string(obj.at("id").as_string()));
        auto name = std::string(obj.at("name").as_string());
        Map map(id, name);
        for (const auto& road_value : obj.at("roads").as_array())
        {
            map.AddRoad(value_to<Road>(road_value));
        }

        for (const auto& building_value : obj.at("buildings").as_array())
        {
            map.AddBuilding(value_to<Building>(building_value));
        }

        for (const auto& office_value : obj.at("offices").as_array())
        {
            map.AddOffice(value_to<Office>(office_value));
        }

        return map;
    }

private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

    Id id_;
    std::string name_;
    Roads roads_;
    Buildings buildings_;

    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
};

class Game
{
public:
    using Maps = std::vector<Map>;

    void AddMap(Map map);

    const Maps& GetMaps() const noexcept { return maps_; }

    const Map* FindMap(const Map::Id& id) const noexcept
    {
        if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end())
        {
            return &maps_.at(it->second);
        }
        return nullptr;
    }

private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

    std::vector<Map> maps_;
    MapIdToIndex map_id_to_index_;
};

}  // namespace model
