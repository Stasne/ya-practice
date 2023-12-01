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
                                                               actions::Right, actions::Stop};
}

using SpeedUnit          = double;
using PlayerPosDimension = double;

struct SpeedVals {
    SpeedUnit vert{0}, hor{0};
    bool      IsNull() const { return vert == 0 && hor == 0; }
    auto      operator<=>(const SpeedVals&) const = default;
};

class Dog : public std::enable_shared_from_this<Dog> {
public:
    Dog(std::string_view name, uint32_t id)
        : name_(name), id_(id), pos_{0, 0}, speed_({0.0, 0.0}), dir_{std::string(actions::Up)}, isActive_(false) {}

    uint32_t         Id() const { return id_; }
    std::string_view GetName() const { return name_; }

    RealPoint        Position() const { return pos_; }
    const RealPoint& SetPosition(const RealPoint& pos) {
        pos_ = pos;
        return pos_;
    }

    SpeedVals Speed() const { return speed_; }
    void      SetSpeed(double speed = 0) {
        isActive_ = (speed != 0);
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

    std::string_view Direction() const { return *dir_; }
    void             SetDirection(game::DogDirection action, double speed = 0) {
        isActive_ = true;
        if (*action == actions::Stop) {
            SetSpeed(0);
            return;
        }
        dir_ = action;
        SetSpeed(speed);
    }

    RealPoint EstimatePosition(uint32_t ticks_ms) const {
        if (speed_.IsNull())
            return pos_;
        RealPoint estimatingPoint;
        double    horPath  = speed_.hor * (static_cast<double>(ticks_ms) / 1000);
        double    vertPath = speed_.vert * (static_cast<double>(ticks_ms) / 1000);
        estimatingPoint.x  = pos_.x + horPath;
        estimatingPoint.y  = pos_.y + vertPath;
        return estimatingPoint;
    }

    bool IsActive() const { return isActive_; }
    bool operator==(const Dog& r) const {
        return id_ == r.id_ && name_ == r.name_ && speed_ == r.speed_ && pos_ == r.pos_ && dir_ == r.dir_;
    }

private:
    uint32_t           id_;
    std::string        name_;
    SpeedVals          speed_;
    RealPoint          pos_;
    game::DogDirection dir_;
    bool               isActive_;
};

using spDog = std::shared_ptr<Dog>;

}  // namespace game