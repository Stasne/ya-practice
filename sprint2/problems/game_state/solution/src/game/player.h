#pragma once
#include <token_machine.h>
#include <memory>
#include <string>
#include <string_view>
namespace game {
class GameSession;
namespace misc_id {
static uint32_t Dog_id{0};
static uint32_t Player_id{0};

}  // namespace misc_id

using Token = security::token::Token;
class Dog : public std::enable_shared_from_this<Dog> {
public:
    Dog(std::string_view name) : name_(name), id_(misc_id::Dog_id++) {}

    uint32_t Id() const { return id_; }

private:
    std::string name_;
    uint32_t id_;
};

using spToken = std::shared_ptr<Token>;
using spDog = std::shared_ptr<Dog>;

class Player : public std::enable_shared_from_this<Player> {
public:
    Player(std::string_view name, spToken token)
        : dog_(std::make_shared<game::Dog>(name)), token_(token), name_(name), id_(misc_id::Player_id++) {}
    std::string_view Name() const { return name_; }
    uint32_t Id() const { return id_; }
    spDog Dog() const { return dog_; }

private:
    std::shared_ptr<GameSession> session_;
    std::string name_;
    uint32_t id_;
    spDog dog_;
    spToken token_;
};
}  // namespace game