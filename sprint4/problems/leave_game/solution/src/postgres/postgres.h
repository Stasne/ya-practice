#pragma once
#include <highscorer.h>
#include <pqxx/connection>
#include <pqxx/transaction>

namespace postgres {

class HighscoreRepositoryImpl : public game::IHighscoreRepository {
public:
    explicit HighscoreRepositoryImpl(pqxx::connection& write, pqxx::connection& read)
        : read_connection_{read}, write_connection_{write} {}

    void WriteResult(game::GameResult&& result) override;

    std::vector<game::GameResult> LoadResults(uint32_t count, uint32_t offset) override;
    void                          Prepare();

private:
    pqxx::connection& write_connection_;
    pqxx::connection& read_connection_;
};

class Database {
public:
    explicit Database(pqxx::connection writer, pqxx::connection reader);

    game::IHighscoreRepository& GetHighScoresHandler() & { return highscores_; }

private:
    pqxx::connection        write_connection_;
    pqxx::connection        read_connection_;
    HighscoreRepositoryImpl highscores_{write_connection_, read_connection_};
};

}  // namespace postgres