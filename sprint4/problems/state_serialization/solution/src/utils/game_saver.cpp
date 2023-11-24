#include "game_saver.h"
#include <game_serialization.h>
#include <logger.h>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <filesystem>
#include <fstream>
namespace fs        = std::filesystem;
using InputArchive  = boost::archive::text_iarchive;
using OutputArchive = boost::archive::text_oarchive;

namespace {
static std::string SaveText("Game saved to file: ");
static std::string LoadText("Game loaded from file: ");
}  // namespace

GameSaver::GameSaver(model::Game& game, std::string_view file_path, std::optional<uint32_t> autosave_period_ms)
    : game_(game),
      autoSavePeriod_ms_(autosave_period_ms ? std::chrono::milliseconds(*autosave_period_ms)
                                            : std::chrono::milliseconds(0)),
      targetStateFile_(file_path),
      activeStateFile_(file_path),
      autosavingEnabled(autosave_period_ms.has_value()) {

    if (!file_path.empty()) {
        LoadText.append(targetStateFile_);
        SaveText.append(targetStateFile_);

        std::ifstream istateStream(targetStateFile_);
        if (istateStream.good()) {
            LoadGame();
            savingEnabled_ = true;
        }
        activeStateFile_ = fs::path(std::string(file_path) + "_active"s);

        std::ofstream ostateStream(activeStateFile_);
        if (ostateStream.good()) {
            savingEnabled_ = true;
        }
    }
}

void GameSaver::TimeTicked(std::chrono::milliseconds delta) {
    if (!autosavingEnabled || !savingEnabled_)  // no autosave
        return;

    msSinceLastSave_ += delta;
    if (msSinceLastSave_ >= autoSavePeriod_ms_) {
        SaveGame();
        msSinceLastSave_ = std::chrono::milliseconds(0);
    }
}
void GameSaver::LoadGame() {
    std::ifstream           infile(targetStateFile_);
    InputArchive            ia{infile};
    serialization::GameRepr repr;
    ia >> repr;
    repr.RestoreGame(game_);
    Logger::Message(LoadText);
}

void GameSaver::SaveGame() {
    if (!savingEnabled_)
        return;
    serialization::GameRepr repr{game_};
    std::ofstream           stateStream(activeStateFile_);
    OutputArchive           oa{stateStream};
    oa << repr;
    stateStream.flush();
    stateStream.close();
    fs::rename(activeStateFile_, targetStateFile_);
    Logger::Message(SaveText);
}

GameSaver::~GameSaver() {
    if (!savingEnabled_)
        return;

    SaveGame();
}