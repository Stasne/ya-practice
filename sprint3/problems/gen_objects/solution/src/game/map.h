#pragma once
#include <tagged.h>
#include <boost/json.hpp>
#include <boost/json/value_to.hpp>
#include <filesystem>
#include <optional>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

namespace model {

struct Loot {
    std::string name;
    std::filesystem::path file;
    std::string type;  // string?
    uint32_t rotation;
    std::string color;  //string?
    double scale;
};

using Dimension = int;
using Coord = Dimension;
using Real = double;

struct Point {
    Coord x, y;
};
struct RealPoint {
    Real x, y;
    double VectorLength(const RealPoint& point) const {
        double squared_distance = pow(point.x - x, 2) + pow(point.y - y, 2);
        return std::sqrt(squared_distance);
    }

    bool operator<=(const RealPoint& other) const { return x <= other.x && y >= other.y; }
    bool operator>=(const RealPoint& other) const { return x >= other.x && y <= other.y; }
    bool operator==(const RealPoint& other) const {
        return (fabs(x - other.x) < std::numeric_limits<double>::epsilon() &&
                fabs(y - other.y) < std::numeric_limits<double>::epsilon());
    }
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

static const double RoadWidth{0.4};
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

    Road(HorizontalTag, Point start, Coord end_x) noexcept
        : width_(RoadWidth),
          start_{start},
          end_{end_x, start.y},
          lbotX_(std::min(start_.x, end_.x) - width_),
          lbotY_(std::max(start_.y, end_.y) + width_),
          rtopX_(std::max(start_.x, end_.x) + width_),
          rtopY_(std::min(start_.y, end_.y) - width_) {}
    Road(VerticalTag, Point start, Coord end_y) noexcept
        : width_(RoadWidth),
          start_{start},
          end_{start.x, end_y},
          lbotX_(std::min(start_.x, end_.x) - width_),
          lbotY_(std::max(start_.y, end_.y) + width_),
          rtopX_(std::max(start_.x, end_.x) + width_),
          rtopY_(std::min(start_.y, end_.y) - width_) {}

    bool IsHorizontal() const noexcept { return start_.y == end_.y; }
    bool IsVertical() const noexcept { return start_.x == end_.x; }
    Point GetStart() const noexcept { return start_; }
    Point GetEnd() const noexcept { return end_; }
    RealPoint GetLeftBotCorner() const noexcept { return {lbotX_, lbotY_}; }
    RealPoint GetRightTopCorner() const noexcept { return {rtopX_, rtopY_}; }

    bool ContainsPoint(const RealPoint& p) const noexcept {
        return (p >= RealPoint(lbotX_, lbotY_) && p <= RealPoint(rtopX_, rtopY_));
    }
    RealPoint FitPointToRoad(const RealPoint& point) const {
        auto boundPoint = point;
        boundPoint.x = std::clamp(boundPoint.x, lbotX_, rtopX_);
        boundPoint.y = std::clamp(boundPoint.y, lbotY_, rtopY_);
        return boundPoint;
    }

private:
    double width_;
    Point start_;
    Point end_;
    const double lbotX_;
    const double lbotY_;
    const double rtopX_;
    const double rtopY_;
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
    using Loots = std::vector<Loot>;

    Map(Id id, std::string name, std::optional<double> dogSpeed = {}) noexcept
        : id_(std::move(id)), name_(std::move(name)), speed_(dogSpeed) {}

    const Id& GetId() const noexcept { return id_; }
    const std::string& GetName() const noexcept { return name_; }
    std::optional<double> GetMapSpeed() const { return speed_; }
    const Buildings& GetBuildings() const noexcept { return buildings_; }
    const Roads& GetRoads() const noexcept { return roads_; }
    const Offices& GetOffices() const noexcept { return offices_; }
    const Loots& GetLootTypes() const noexcept { return loot_; }
    void AddRoad(const Road& road) { roads_.emplace_back(road); }
    void AddBuilding(const Building& building) { buildings_.emplace_back(building); }
    void AddOffice(Office office);
    void AddLoot(Loot loot) { loot_.emplace_back(loot); }
    Roads GetRoadsForPoint(const RealPoint& p) const noexcept;

private:
    Id id_;
    std::string name_;
    std::optional<double> speed_;
    Roads roads_;
    Buildings buildings_;
    Loots loot_;

    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;
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
void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, const Loot& building);
Loot tag_invoke(boost::json::value_to_tag<Loot>, const boost::json::value& jv);
}  //namespace model