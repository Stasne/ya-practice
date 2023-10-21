#pragma once
#include <tagged.h>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_set>
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
struct GameActionTag {};
}  // namespace detail
using GameAction = util::Tagged<std::string, detail::GameActionTag>;
namespace misc {
static const std::unordered_set<std::string_view> ValidActions{actions::Up, actions::Down, actions::Left,
                                                               actions::Right};
}

using SpeedUnit = float;
using PlayerPosDimension = float;

struct SpeedVals {
    SpeedUnit vert{0}, hor{0};
};
struct PlayerPoint {
    PlayerPosDimension x{0}, y{0};
};

class Dog : public std::enable_shared_from_this<Dog> {
public:
    Dog(std::string_view name, uint32_t id) : name_(name), id_(id /*misc_id::Dog_id++*/), speed_({0.0, 0.0}) {}
    PlayerPoint Position() const { return pos_; }
    PlayerPoint SetPosition(const PlayerPoint& pos) {  //check if position is correct
        pos_ = pos;
        return pos_;
    }
    SpeedVals Speed() const { return speed_; }
    uint32_t Id() const { return id_; }
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
    void Move(game::GameAction action, double speed) {
        if (*action == game::actions::Stop) {
            speed_.hor = 0;
            speed_.vert = 0;
        }
        if (*action == game::actions::Up) {
            speed_.hor = 0;
            speed_.vert = speed;
        }
        if (*action == game::actions::Down) {
            speed_.hor = 0;
            speed_.vert = -speed;
        }
        if (*action == game::actions::Left) {
            speed_.hor = -speed;
            speed_.vert = 0;
        }
        if (*action == game::actions::Right) {
            speed_.hor = speed;
            speed_.vert = 0;
        }
    }

private:
private:
    uint32_t id_;
    std::string name_;
    SpeedVals speed_;
    PlayerPoint pos_;
};
using spDog = std::shared_ptr<Dog>;

}  // namespace game