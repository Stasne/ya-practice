#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>

using namespace std::literals;
namespace net = boost::asio;

namespace game {

struct GameResult {
    std::string player;
    uint32_t    score;
    double      playTime_s;
};

class IHighscoreRepository {
public:
    virtual void                    WriteResult(GameResult&& result)         = 0;
    virtual std::vector<GameResult> LoadResults(uint32_t count = 100, uint32_t offset = 0,
                                                std::string_view name = ""s) = 0;
};

using ExecutionStrand = net::strand<net::io_context::executor_type>;

class Highscorer final {
public:
    Highscorer(ExecutionStrand& strand, IHighscoreRepository& repo) : strand_(strand), repository_(repo) {}

    void UpdateHighScore(GameResult&& result) {
        // TODO: HELP me to figure out why is this doesnt work?
        //    net::post(strand_, [row = std::move(result), &repo = repository_] { repo.WriteResult(std::move(row)); });
        // chat-gpt made next line instead...
        net::post(strand_, [row = std::move(result), repo = std::ref(repository_)]() mutable {
            repo.get().WriteResult(std::move(row));
        });
    }

    std::vector<GameResult> LoadHighScores(uint32_t count = 100, uint32_t offset = 0, std::string_view name = ""s) {
        return repository_.LoadResults(count, offset, name);
    };

private:
    IHighscoreRepository& repository_;
    ExecutionStrand&      strand_;
};

}  // namespace game