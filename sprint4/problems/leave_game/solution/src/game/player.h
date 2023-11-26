#pragma once
#include <model/dog.h>
#include <token_machine.h>
#include <memory>
#include <string>
#include <string_view>
#include "game.h"

namespace game {

using namespace std::literals;

using Token         = security::token::Token;
using spToken       = std::shared_ptr<Token>;
using spGameSession = std::shared_ptr<GameSession>;

class Player : public std::enable_shared_from_this<Player> {
public:
    Player(spDog doge, spToken token) : id_(doge->Id()), name_(doge->GetName()), dog_(doge), token_(token) {}
    Player(std::string_view name, spToken token, uint32_t id)
        : dog_(std::make_shared<game::Dog>(name, id_)), token_(token), name_(name), id_(id) {}

    std::string_view Name() const { return name_; }
    uint32_t         Id() const { return id_; }
    const spDog      GetDog() const { return dog_; }  // naming issue

    static bool IsActionValid(std::string_view action) { return game::misc::ValidActions.count(action); }
    bool        operator==(const Player& rhs) const {
        if (!dog_ || !rhs.dog_)
            return false;
        return id_ == rhs.id_ && name_ == rhs.name_ && *token_ == *rhs.token_ && *dog_ == *rhs.dog_;
    }

private:
    uint32_t    id_;
    std::string name_;
    spToken     token_;
    spDog       dog_;
};
}  // namespace game