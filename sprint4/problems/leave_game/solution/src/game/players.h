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

    Players() = default;

    void     AddPlayer(spPlayer player, spToken token) { InsertPlayer(player, token); }
    spPlayer NewPlayer(std::string_view playerName, spToken token) {

        auto player = std::make_shared<Player>(playerName, token, GetAvailablPlayerIndex());
        InsertPlayer(player, token);

        return player;
    };
    void RemovePlayer(uint32_t pid) {}

    const PlayersCollection& PlayersMap() const { return players_; }

    spPlayer PlayerById(uint32_t id) const {
        if (!players_.count(id))
            return {};
        return players_.at(id);
    }

    std::weak_ptr<Player> PlayerByDog(const Dog& doge) {
        if (!players_.count(doge.Id()))
            return {};
        return players_.at(doge.Id());
    };

    std::weak_ptr<Player> PlayerByToken(const Token& token) {
        if (!playersByToken_.count(*token))
            return {};
        return playersByToken_.at(*token);
    }

    const PlayersByToken& PlayersCredits() const { return playersByToken_; }

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
    void InsertPlayer(spPlayer player, spToken token) {
        players_[player->Id()]         = player;
        playersByToken_[**token.get()] = player;  //key shared/weak? i dunno. by value yet
    }
    void RemovePlayer(spPlayer player) {
        players_.erase(player->Id());
        playersByToken_.erase(*player->TokenView());
    }
    uint32_t GetAvailablPlayerIndex() const {
        static uint32_t Player_id{0};
        while (players_.count(Player_id))
            ++Player_id;

        return Player_id;
    }

private:
    // это позорище какое-то, но писать оператор сравнения для собак не хотелось... hmmm
    PlayersCollection players_;
    PlayersByToken    playersByToken_;
};
}  // namespace game