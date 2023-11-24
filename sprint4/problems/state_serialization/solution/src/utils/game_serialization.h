#include <game.h>
#include <token_machine.h>
#include <boost/serialization/map.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>
namespace model {

template <typename Archive>
void serialize(Archive& ar, RealPoint& obj, [[maybe_unused]] const unsigned version) {
    ar&(obj.x);
    ar&(obj.y);
}
template <typename Archive>
void serialize(Archive& ar, MapLoot& obj, [[maybe_unused]] const unsigned version) {
    ar&(obj.pos);
    ar&(obj.type);
}

}  // namespace model

namespace game {

template <typename Archive>
void serialize(Archive& ar, BagSlot& obj, [[maybe_unused]] const unsigned version) {
    ar&(obj.item_id);
    ar&(obj.type);
}
}  // namespace game
namespace serialization {

// DogRepr (DogRepresentation) - сериализованное представление класса Dog
class DogRepr {
public:
    DogRepr() = default;

    explicit DogRepr(const game::Dog& dog) : id_(dog.Id()), name_(dog.GetName()), pos_(dog.Position()) {}

    [[nodiscard]] game::spDog Restore() const {
        auto dog = std::make_shared<game::Dog>(name_, id_);
        dog->SetPosition(pos_);
        return dog;
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar & id_;
        ar & name_;
        ar & pos_;
    }

private:
    // saving parameters
    uint32_t         id_;
    std::string      name_;
    model::RealPoint pos_;
};

class PlayersHandlerRepr {
public:
    PlayersHandlerRepr() = default;

    explicit PlayersHandlerRepr(const game::Players& ph) {
        for (const auto& [token, spPlayer] : ph.PlayersCredits()) {
            credits_[token] = DogRepr(*spPlayer->GetDog());
        }
    }

    [[nodiscard]] game::Players Restore() const {
        game::Players players;
        for (const auto& [token, dog] : credits_) {
            auto restoredToken  = security::token::RestoreToken(token);  //spToken
            auto doge           = dog.Restore();                         //spDog
            auto restoredPlayer = std::make_shared<Player>(doge, restoredToken);
            players.AddPlayer(restoredPlayer, restoredToken);
        }

        return players;
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar & credits_;
    }

private:
    // saving parameters
    std::unordered_map<std::string /*token*/, DogRepr> credits_;
};

class PlayingUnitRepr {
public:
    PlayingUnitRepr() = default;
    explicit PlayingUnitRepr(const game::PlayingUnit& p) : dogId(p.dog->Id()), bag(p.bag), score(p.score) {}
    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar & dogId;
        ar & bag;
        ar & score;
    }

    uint32_t  dogId;
    game::Bag bag;
    uint32_t  score;
};

class SessionRepr {
public:
    SessionRepr() = default;
    explicit SessionRepr(const game::GameSession& gs)
        : mapName(gs.GetMap().GetName()), sessionName(gs.GetName()), sid(gs.GetId()) {
        for (const auto& [id, pl] : gs.GetPlayers())
            playersRepr[id] = PlayingUnitRepr(pl);

        mapLootRepr = gs.GetLoot();
    }
    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar & sid;
        ar & mapName;
        ar & sessionName;
        ar & playersRepr;
        ar & mapLootRepr;
    }

    uint32_t                                      sid;
    std::string                                   mapName;
    std::string                                   sessionName;
    std::unordered_map<uint32_t, PlayingUnitRepr> playersRepr;
    game::GameSession::LootPositions              mapLootRepr;
};

class GameRepr {
public:
    GameRepr() = default;
    explicit GameRepr(model::Game& g) : playersHandlerRepr_(g.PlayersHandler()) {
        for (const auto& [id, spSession] : g.GetGameSessions()) {
            sessionsRepr_[id] = SessionRepr(*spSession);
        }
    }

    void RestoreGame(model::Game& game) const {
        const auto& maps      = game.GetMaps();
        game.PlayersHandler() = playersHandlerRepr_.Restore();
        // for every session
        for (const auto& [id, sr] : sessionsRepr_) {
            // find map
            auto mapIt =
                std::find_if(maps.cbegin(), maps.cend(), [&](const auto& map) { return map.GetName() == sr.mapName; });
            if (mapIt == maps.cend())
                throw std::runtime_error("Restoring game error: unknown mapname" + sr.mapName);

            auto session = game.StartGame(*mapIt, sr.sessionName, sr.sid);
            session->RestoreMapLoot(sr.mapLootRepr);
            for (const auto& [id, p] : sr.playersRepr) {
                auto doge = game.PlayersHandler().PlayerById(id)->GetDog();
                session->AddDog(doge);
                session->RestorePlayerLoot(id, p.bag, p.score);
            }
        }
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar & sessionsRepr_;
        ar & playersHandlerRepr_;
    }

private:
    std::unordered_map<uint32_t, SessionRepr> sessionsRepr_;
    PlayersHandlerRepr                        playersHandlerRepr_;
};

}  // namespace serialization