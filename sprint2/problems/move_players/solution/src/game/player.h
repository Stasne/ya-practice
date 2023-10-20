#pragma once
#include <dog.h>
#include <game.h>
#include <token_machine.h>
#include <memory>
#include <string>
#include <string_view>

namespace game {

namespace misc {
// static uint32_t Dog_id{0}; // lets make same with player
static uint32_t Player_id{0};

}  // namespace misc

using namespace std::literals;

using Token = security::token::Token;
using spToken = std::shared_ptr<Token>;
using spGameSession = std::shared_ptr<model::GameSession>;

class Player : public std::enable_shared_from_this<Player> {
public:
    Player(std::string_view name, spToken token)
        : dog_(std::make_shared<game::Dog>(name, id_)), token_(token), name_(name), id_(misc::Player_id++) {}
    std::string_view Name() const { return name_; }
    uint32_t Id() const { return id_; }
    const Dog& GetDog() const { return *dog_; }  // naming issue
    void AssignGame(spGameSession session) { session_ = session; }
    spGameSession CurrentGame() const { return session_; }

    static bool IsActionValid(std::string_view action) { return game::misc::ValidActions.count(action); }

private:
    uint32_t id_;
    std::string name_;
    spToken token_;
    spGameSession session_;
    spDog dog_;
};
}  // namespace game