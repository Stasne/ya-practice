#pragma once
#include <memory>
#include <string>
#include <string_view>

namespace game {

using namespace std::literals;

using SpeedUnit = float;
using PlayerPosDimension = float;

struct SpeedVals {
    SpeedUnit vert, hor;
};
struct PlayerPoint {
    PlayerPosDimension x, y;
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

private:
    uint32_t id_;
    std::string name_;
    SpeedVals speed_;
    PlayerPoint pos_;
};
using spDog = std::shared_ptr<Dog>;

}  // namespace game