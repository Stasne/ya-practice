#pragma once
#include <token_machine.h>
#include <boost/algorithm/string.hpp>
#include "player.h"
using Token = security::token::Token;
using spToken = std::shared_ptr<Token>;
namespace game {

class Players {
public:
    using PlayersCollection = std::unordered_map<uint32_t /*id*/, std::shared_ptr<Player>>;
    using PlayersByToken = std::unordered_map<std::string /*token as string*/, std::shared_ptr<Player>>;
    Players() = default;

    std::shared_ptr<Player> NewPlayer(std::string_view playerName, spToken token) {
        if (!ValidatePlayerName(playerName))
            return nullptr;

        auto player = std::make_shared<Player>(playerName, token);
        players_[player->Id()] = player;
        playerByToken_[**token.get()] = player;  //shared/weak? i dunno. by value yet

        return player;
    };
    const PlayersCollection& PlayersMap() const { return players_; }
    // std::weak_ptr<Player> Player(/*Id*/);
    // std::weak_ptr<Player> Player(/*Dog*/);
    // std::weak_ptr<Player> Player(/*Token*/);
private:
    bool ValidatePlayerName(std::string_view playerName) const {
        return std::find_if(players_.begin(), players_.end(), [playerName](auto&& player) {
                   return boost::iequals(player.second->Name(), playerName);
               }) == players_.end();
    }

private:
    PlayersCollection players_;
    PlayersByToken playerByToken_;
};
}  // namespace game