#pragma once
#include <game.h>

class GameSaver : public model::IGameListener {
public:
    GameSaver(model::Game& game, std::string_view file_path, std::optional<uint32_t> autosave_period_ms);
    void SaveGame();
    void LoadGame();
    ~GameSaver();

private:
    void TimeTicked(std::chrono::milliseconds delta) override;

private:
    model::Game&              game_;
    std::chrono::milliseconds msSinceLastSave_;
    std::chrono::milliseconds autoSavePeriod_ms_;
    std::filesystem::path     stateFile_;
    bool                      savingEnabled_{false};
    bool                      autosavingEnabled{false};
};
