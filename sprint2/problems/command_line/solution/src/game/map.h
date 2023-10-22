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
static double bound(const double first, const double second, const double value) {
    double lower = std::min(first, second);
    double upper = std::max(first, second);

    double result;
    result = std::min(upper, value);
    result = std::max(lower, result);
    return result;
}
static uint32_t bound(const uint32_t first, const uint32_t second, const uint32_t value) {
    uint32_t lower = std::min(first, second);
    uint32_t upper = std::max(first, second);

    uint32_t result;
    result = std::min(upper, value);
    result = std::max(lower, result);
    return result;
}
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
          rtopY_(std::min(start_.y, end_.y) - width_) {
        auto i = 0;
    }
    Road(VerticalTag, Point start, Coord end_y) noexcept
        : width_(RoadWidth),
          start_{start},
          end_{start.x, end_y},
          lbotX_(std::min(start_.x, end_.x) - width_),
          lbotY_(std::max(start_.y, end_.y) + width_),
          rtopX_(std::max(start_.x, end_.x) + width_),
          rtopY_(std::min(start_.y, end_.y) - width_) {
        auto i = 0;
    }

    bool IsHorizontal() const noexcept { return start_.y == end_.y; }
    bool IsVertical() const noexcept { return start_.x == end_.x; }

    Point GetStart() const noexcept { return start_; }
    Point GetEnd() const noexcept { return end_; }
    bool ContainsPoint(const game::PlayerPoint& p) const noexcept {
        return (p >= game::PlayerPoint(lbotX_, lbotY_) && p <= game::PlayerPoint(rtopX_, rtopY_));
    }
    game::PlayerPoint FitPointToRoad(const game::PlayerPoint& nextPoint) const noexcept {
        auto boundPoint = nextPoint;
        boundPoint.x = bound(lbotX_, rtopX_, boundPoint.x);
        boundPoint.y = bound(lbotY_, rtopY_, boundPoint.y);
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

    Map(Id id, std::string name, std::optional<double> dogSpeed = {}) noexcept
        : id_(std::move(id)), name_(std::move(name)), speed_(dogSpeed) {}

    const Id& GetId() const noexcept { return id_; }
    const std::string& GetName() const noexcept { return name_; }
    std::optional<double> GetMapSpeed() const { return speed_; }
    const Buildings& GetBuildings() const noexcept { return buildings_; }
    const Roads& GetRoads() const noexcept { return roads_; }
    const Offices& GetOffices() const noexcept { return offices_; }
    game::PlayerPoint GetSpawnPoint(bool isRandom) const {
        if (!roads_.size())
            return {0, 0};
        if (!isRandom) {
            return {static_cast<double>(roads_.front().GetStart().x), static_cast<double>(roads_.front().GetStart().y)};
        }

        static uint32_t seed{0};
        auto& chosenRoad = roads_[seed++ % roads_.size()];

        return {static_cast<double>(bound(chosenRoad.GetStart().x, chosenRoad.GetEnd().x, seed)),
                static_cast<double>(bound(chosenRoad.GetStart().y, chosenRoad.GetEnd().y, seed))};
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