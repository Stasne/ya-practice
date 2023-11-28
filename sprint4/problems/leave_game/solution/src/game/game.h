#pragma once
#include <highscorer.h>
#include <model/map.h>
#include <chrono>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "game_session.h"
#include "players.h"

using namespace game;
namespace model {

namespace defaults {
static const uint32_t BAG_CAPACITY{3};
static const double   ITEM_WIDTH{0.0};
static const double   DOG_WIDTH{0.6};
static const double   OFFICE_WIDTH{0.5};  // База/оффис/пункт приёма
static const double   ITEM_GENERATION_PERIOD{5};
static const double   ITEM_GENERATION_PROBABILITY{0.5};
}  // namespace defaults

class IGameListener {
public:
    virtual void TimeTicked(GameTimePeriod delta) = 0;
};

class Game {
public:
    // Maps workflow
    using Maps         = std::vector<Map>;
    using GameSessions = std::unordered_map<uint32_t, spGameSession>;

    void        AddMap(Map map);
    const Maps& GetMaps() const noexcept { return maps_; }
    const Map*  FindMap(const Map::Id& id) const noexcept {
        if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
            return &maps_.at(it->second);
        }
        return nullptr;
    }

    // Players controller
    Players& PlayersHandler() { return players_; }

    // Gaming sessions
    spGameSession StartGame(const Map& map, std::string_view name = "", std::optional<uint32_t> id = std::nullopt);
    const spGameSession FindGame(uint32_t dogId) {
        for (const auto& [_, session] : sessions_) {
            if (session->GetPlayers().count(dogId))
                return session;
        }
        return {};
    }

    void TickTime(uint32_t tick_ms) {
        for (const auto& [_, session] : sessions_) {
            session->UpdateState(GameTimePeriod(tick_ms));
            auto leftPlayers = session->GetLeftPlayers();
            for (const auto& [id, leftPlayer] : leftPlayers) {
                // if (leftPlayer.score)
                UpdateHighScore(leftPlayer);
            }
        }

        for (auto& listener : listeners_)
            listener->TimeTicked(GameTimePeriod(tick_ms));
    }

    void SetHighScoreHandler(std::shared_ptr<Highscorer> hs) { highscore_ = hs; }
    void SetRandomSpawnEnabled(bool isEnabled) { randomSpawn_ = isEnabled; }
    void SetDefaultDogSpeed(double speed) { defaultSpeed_ = speed; }
    void SetDefaultBagCapacity(uint32_t capacity) { defaultBagCapacity_ = capacity; }
    void SetRandomSpawnPoint(bool isRandom = false) { randomSpawn_ = isRandom; }
    void SetDogAfkTimeout(double kick_timeout_seconds) { afkKickTimeout_ = kick_timeout_seconds; }
    void SetRandomGeneratorConfig(double period      = defaults::ITEM_GENERATION_PERIOD,
                                  double probability = defaults::ITEM_GENERATION_PROBABILITY) {
        randomGeneratorPeriod_      = period;
        randomGeneratorProbability_ = probability;
    }
    const GameSessions&         GetGameSessions() const { return sessions_; }
    std::shared_ptr<Highscorer> GetHighScoreHandler() const { return highscore_; }

    void AddListener(IGameListener* listener) { listeners_.push_back(listener); }

private:
    void UpdateHighScore(const game::PlayingUnit& unit) {
        GameResult highscore{.player     = std::string(unit.dog->GetName()),
                             .score      = unit.score,
                             .playTime_s = unit.play_time.count() / 1000.0};
        highscore_->UpdateHighScore(std::move(highscore));
        int i = 20;
    };
    void ValidateSessions() {
        // check if session is have to be finished
    }

private:
    using MapIdHasher  = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;
    std::shared_ptr<Highscorer> highscore_;
    GameSessions                sessions_;
    Maps                        maps_;
    MapIdToIndex                map_id_to_index_;
    Players                     players_;
    double                      defaultSpeed_;
    uint32_t                    defaultBagCapacity_{defaults::BAG_CAPACITY};
    double                      defaultItemWidth{defaults::ITEM_WIDTH};
    double                      defaultDogWidth{defaults::DOG_WIDTH};
    double                      defaultOfficeWidth{defaults::OFFICE_WIDTH};
    double                      afkKickTimeout_{10.0};
    bool                        randomSpawn_ = false;
    uint32_t                    randomGeneratorPeriod_;
    double                      randomGeneratorProbability_;
    // "Слушатели" тиков игры
    std::vector<IGameListener*> listeners_;
};

}  // namespace model
