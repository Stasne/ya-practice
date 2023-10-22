#pragma once
#include <logger.h>
#include <tagged.h>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_set>

/*
    std::string_view Direction() const {
        if (speed_.vert == 0 && speed_.hor == 0) {
            return "U"sv;  // Игрок не двигается
        } else if (speed_.vert > 0) {
            return "N"sv;  // Движение на север
        } else if (speed_.hor > 0) {
            return "E"sv;  // Движение на восток
        } else if (speed_.hor < 0) {
            return "W"sv;  // Движение на запад
        } else {
            return "S"sv;  // Движение на юг
        }
    }
*/
namespace game {
using namespace std::literals;
namespace actions {
static constexpr std::string_view Stop{""sv};
static constexpr std::string_view Up{"U"sv};
static constexpr std::string_view Down{"D"sv};
static constexpr std::string_view Right{"R"sv};
static constexpr std::string_view Left{"L"sv};
}  // namespace actions
namespace detail {
struct DirectionTag {};
}  // namespace detail
using DogDirection = util::Tagged<std::string, detail::DirectionTag>;
namespace misc {
static const std::unordered_set<std::string_view> ValidActions{actions::Up, actions::Down, actions::Left,
                                                               actions::Right};
}

using SpeedUnit = double;
using PlayerPosDimension = double;

struct SpeedVals {
    SpeedUnit vert{0}, hor{0};
};
struct PlayerPoint {
    PlayerPosDimension x{0}, y{0};

    double VectorLength(const PlayerPoint& point) const {
        double squared_distance = pow(point.x - x, 2) + pow(point.y - y, 2);
        return std::sqrt(squared_distance);
    }

    bool operator<=(const PlayerPoint& other) const { return x <= other.x && y >= other.y; }
    bool operator>=(const PlayerPoint& other) const { return x >= other.x && y <= other.y; }
    bool operator==(const PlayerPoint& other) const {
        return (fabs(x - other.x) < std::numeric_limits<double>::epsilon() &&
                fabs(y - other.y) < std::numeric_limits<double>::epsilon());
    }
};

class Dog : public std::enable_shared_from_this<Dog> {
public:
    Dog(std::string_view name, uint32_t id)
        : name_(name), id_(id /*misc_id::Dog_id++*/), pos_{0, 0}, speed_({0.0, 0.0}), dir_{std::string(actions::Up)} {}
    PlayerPoint Position() const { return pos_; }
    const PlayerPoint& SetPosition(const PlayerPoint& pos) {  //check if position is correct
        boost::json::value json{{"dog", name_}, {"x", pos_.x}, {"y", pos_.y}};
        BOOST_LOG_TRIVIAL(debug) << boost::log::add_value(additional_data, json) << "SetPosition";
        pos_ = pos;
        return pos_;
    }
    PlayerPoint EstimatePosition(uint32_t ticks_ms) const {
        PlayerPoint estimatingPoint;
        double horPath = speed_.hor * (static_cast<double>(ticks_ms) / 1000);
        double vertPath = speed_.vert * (static_cast<double>(ticks_ms) / 1000);
        estimatingPoint.x = pos_.x + horPath;
        estimatingPoint.y = pos_.y + vertPath;
        return estimatingPoint;
    }

    SpeedVals Speed() const { return speed_; }
    uint32_t Id() const { return id_; }
    std::string_view Direction() const { return *dir_; }
    void SetSpeed(double speed = 0) {
        if (*dir_ == game::actions::Stop) {
            speed_.hor = 0;
            speed_.vert = 0;
        }
        if (*dir_ == game::actions::Up) {
            speed_.hor = 0;
            speed_.vert = -speed;
        }
        if (*dir_ == game::actions::Down) {
            speed_.hor = 0;
            speed_.vert = speed;
        }
        if (*dir_ == game::actions::Left) {
            speed_.hor = -speed;
            speed_.vert = 0;
        }
        if (*dir_ == game::actions::Right) {
            speed_.hor = speed;
            speed_.vert = 0;
        }
    }
    void SetDirection(game::DogDirection action, double speed = 0) {
        dir_ = action;
        SetSpeed(speed);
    }

private:
private:
    uint32_t id_;
    std::string name_;
    SpeedVals speed_;
    PlayerPoint pos_;
    game::DogDirection dir_;
};
using spDog = std::shared_ptr<Dog>;

}  // namespace game