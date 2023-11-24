#pragma once
#include <token_machine.h>
#include <boost/algorithm/string.hpp>
#include "player.h"
namespace game {

using Token    = security::token::Token;
using spToken  = std::shared_ptr<Token>;
using spPlayer = std::shared_ptr<Player>;
class Players {
public:
    using PlayersCollection = std::unordered_map<uint32_t /*id*/, spPlayer>;
    using PlayersByToken    = std::unordered_map<std::string /*token as string*/, spPlayer>;
    using PlayersByDogId    = std::unordered_map<uint32_t, spPlayer>;
    Players()               = default;

    void     AddPlayer(spPlayer player, spToken token) { InsertPlayer(player, token); }
    spPlayer NewPlayer(std::string_view playerName, spToken token) {
        if (!ValidatePlayerName(playerName))
            return nullptr;

        auto player = std::make_shared<Player>(playerName, token);
        InsertPlayer(player, token);

        return player;
    };
    const PlayersCollection& PlayersMap() const { return players_; }
    spPlayer                 PlayerById(uint32_t id) const {
        if (!players_.count(id))
            return {};
        return players_.at(id);
    }
    std::weak_ptr<Player> PlayerByDog(const Dog& doge) {
        if (!playersByDog_.count(doge.Id()))
            return {};
        return playersByDog_.at(doge.Id());
    };

    std::weak_ptr<Player> PlayerByToken(const Token& token) {
        if (!playersByToken_.count(*token))
            return {};
        return playersByToken_.at(*token);
    }
    const PlayersByToken& PlayersCredits() const { return playersByToken_; }

    // bool operator==(const Players& r) const {
    //     if (players_.size() != r.players_.size())
    //         return false;
    //     for (const auto& [id, player] : players_) {
    //         if (r.players_.at(id) != player)
    //             return false;
    //     }
    //     return true;
    // }
    bool operator==(const Players& rhs) const {
        if (players_.size() != rhs.players_.size())
            return false;

        for (const auto& [id, spPlayer] : players_) {
            if (!rhs.players_.count(id))
                return false;
            if (*spPlayer != *rhs.players_.at(id))
                return false;
        }
        return true;
    }

private:
    bool ValidatePlayerName(std::string_view playerName) const {
        return std::find_if(players_.begin(), players_.end(), [playerName](auto&& player) {
                   return boost::iequals(player.second->Name(), playerName);
               }) == players_.end();
    }
    void InsertPlayer(spPlayer player, spToken token) {
        // balya, a eto ne 'too much' 3 collections?
        players_[player->Id()]                = player;
        playersByToken_[**token.get()]        = player;  //key shared/weak? i dunno. by value yet
        playersByDog_[player->GetDog()->Id()] = player;
    }

private:
    // это позорище какое-то, но писать оператор сравнения для собак не хотелось... hmmm
    PlayersCollection players_;
    PlayersByToken    playersByToken_;
    PlayersByDogId    playersByDog_;
};
}  // namespace game