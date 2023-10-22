#pragma once
#include <dog.h>
#include <tagged.h>
#include <boost/json.hpp>
#include <boost/json/value_to.hpp>
#include <optional>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

namespace model {

using Dimension = int;
using Coord = Dimension;

struct Point {
    Coord x, y;
};

struct Size {
    Dimension width, height;
};

struct Rectangle {
    Point position;
    Size size;
};

struct Offset {
    Dimension dx, dy;
};

class Road {
    struct HorizontalTag {
        explicit HorizontalTag() = default;
    };
    struct VerticalTag {
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
    bool ContainsPoint(const game::PlayerPoint& p) const noexcept {
        if (IsHorizontal()) {
            return p.y >= start_.y - halfWidth_ && p.y <= start_.y + halfWidth_ && p.x >= start_.x && p.x <= end_.x;
        } else if (IsVertical()) {
            return p.x >= start_.x - halfWidth_ && p.x <= start_.x + halfWidth_ && p.y >= start_.y && p.y <= end_.y;
        } else {
            return false;
        }
    }
    bool FitPointToRoadMade(game::PlayerPoint& nextPoint) const noexcept {
        if (this->IsHorizontal()) {
            auto big = static_cast<double>(std::max(start_.x, end_.x));
            auto small = static_cast<double>(std::min(start_.x, end_.x));
            nextPoint.x = std::min(std::max(small, nextPoint.x), big);
            double upperBound = static_cast<double>(start_.y + halfWidth_);
            double lowerBound = static_cast<double>(start_.y - halfWidth_);
            nextPoint.y = std::min(std::max(lowerBound, nextPoint.x), upperBound);
        } else {
            auto big = static_cast<double>(std::max(start_.y, end_.y));
            auto small = static_cast<double>(std::min(start_.y, end_.y));
            nextPoint.y = std::min(std::max(small, nextPoint.y), big);
            double rightBound = start_.x + halfWidth_;
            double leftBound = start_.x - halfWidth_;
            nextPoint.x = std::min(std::max(leftBound, nextPoint.x), rightBound);
        }

        return false;
    }

private:
    double halfWidth_{0.4};
    Point start_;
    Point end_;
};

class Building {
public:
    explicit Building(Rectangle bounds) noexcept : bounds_{bounds} {}

    const Rectangle& GetBounds() const noexcept { return bounds_; }

private:
    Rectangle bounds_;
};

class Office {
public:
    using Id = util::Tagged<std::string, Office>;

    Office(Id id, Point position, Offset offset) noexcept : id_{std::move(id)}, position_{position}, offset_{offset} {}

    const Id& GetId() const noexcept { return id_; }

    Point GetPosition() const noexcept { return position_; }
    Offset GetOffset() const noexcept { return offset_; }

private:
    Id id_;
    Point position_;
    Offset offset_;
};

class Map {
public:
    using Id = util::Tagged<std::string, Map>;
    using Roads = std::vector<Road>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;

    Map(Id id, std::string name, std::optional<double> dogSpeed = {}) noexcept
        : id_(std::move(id)), name_(std::move(name)), speed_(dogSpeed) {}

    const Id& GetId() const noexcept { return id_; }
    const std::string& GetName() const noexcept { return name_; }
    std::optional<double> GetMapSpeed() const { return speed_; }
    const Buildings& GetBuildings() const noexcept { return buildings_; }
    const Roads& GetRoads() const noexcept { return roads_; }
    const Offices& GetOffices() const noexcept { return offices_; }
    game::PlayerPoint GetSpawnPoint() const {
        static std::default_random_engine eng{std::random_device{}()};
        std::uniform_int_distribution<std::size_t> road_dist(0, roads_.size() - 1);

        const Road& selected_road = roads_[road_dist(eng)];

        if (selected_road.IsHorizontal()) {
            std::uniform_int_distribution<Coord> coord_dist(selected_road.GetStart().x, selected_road.GetEnd().x);
            return {coord_dist(eng), selected_road.GetStart().y};
        } else if (selected_road.IsVertical()) {
            std::uniform_int_distribution<Coord> coord_dist(selected_road.GetStart().y, selected_road.GetEnd().y);
            return {selected_road.GetStart().x, coord_dist(eng)};
        }

        // Fallthrough case, should never be reached, but included for safety.
        return {0, 0};
    }

    void AddRoad(const Road& road) { roads_.emplace_back(road); }
    void AddBuilding(const Building& building) { buildings_.emplace_back(building); }
    void AddOffice(Office office);
    Roads GetRoadsForPoint(const game::PlayerPoint& p) const noexcept {
        Roads result;
        for (const auto& road : roads_) {
            if (road.ContainsPoint(p)) {
                result.push_back(road);
            }
        }
        return result;
    }

private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

    Id id_;
    std::string name_;
    std::optional<double> speed_;
    Roads roads_;
    Buildings buildings_;

    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
};

void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, const Office& office);
Office tag_invoke(boost::json::value_to_tag<Office>, const boost::json::value& jv);
void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, const Map& map);
Map tag_invoke(boost::json::value_to_tag<Map>, const boost::json::value& jv);
void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, const Road& road);
Road tag_invoke(boost::json::value_to_tag<Road>, const boost::json::value& jv);
void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, const Building& building);
Building tag_invoke(boost::json::value_to_tag<Building>, const boost::json::value& jv);
}  //namespace model