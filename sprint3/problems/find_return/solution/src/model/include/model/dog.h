#pragma once
#include <memory>
#include <string>
#include <string_view>
#include <unordered_set>
#include "map.h"
#include "tagged.h"

using RealPoint = model::RealPoint;

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

using SpeedUnit          = double;
using PlayerPosDimension = double;

struct SpeedVals {
    SpeedUnit vert{0}, hor{0};
};

class Dog : public std::enable_shared_from_this<Dog> {
public:
    Dog(std::string_view name, uint32_t id)
        : name_(name), id_(id /*misc_id::Dog_id++*/), pos_{0, 0}, speed_({0.0, 0.0}), dir_{std::string(actions::Up)} {}
    RealPoint        Position() const { return pos_; }
    const RealPoint& SetPosition(const RealPoint& pos) {  //check if position is correct?
        pos_ = pos;
        return pos_;
    }
    RealPoint EstimatePosition(uint32_t ticks_ms) const {
        RealPoint estimatingPoint;
        double    horPath  = speed_.hor * (static_cast<double>(ticks_ms) / 1000);
        double    vertPath = speed_.vert * (static_cast<double>(ticks_ms) / 1000);
        estimatingPoint.x  = pos_.x + horPath;
        estimatingPoint.y  = pos_.y + vertPath;
        return estimatingPoint;
    }

    SpeedVals        Speed() const { return speed_; }
    uint32_t         Id() const { return id_; }
    std::string_view Direction() const { return *dir_; }
    void             SetSpeed(double speed = 0) {
        if (*dir_ == game::actions::Stop) {
            speed_.hor  = 0;
            speed_.vert = 0;
        }
        if (*dir_ == game::actions::Up) {
            speed_.hor  = 0;
            speed_.vert = -speed;
        }
        if (*dir_ == game::actions::Down) {
            speed_.hor  = 0;
            speed_.vert = speed;
        }
        if (*dir_ == game::actions::Left) {
            speed_.hor  = -speed;
            speed_.vert = 0;
        }
        if (*dir_ == game::actions::Right) {
            speed_.hor  = speed;
            speed_.vert = 0;
        }
    }
    void SetDirection(game::DogDirection action, double speed = 0) {
        dir_ = action;
        SetSpeed(speed);
    }

private:
    uint32_t           id_;
    std::string        name_;
    SpeedVals          speed_;
    RealPoint          pos_;
    game::DogDirection dir_;
};
using spDog = std::shared_ptr<Dog>;

}  // namespace game