#include "postgres.h"
#include <iostream>
#include <pqxx/pqxx>
#include <pqxx/zview.hxx>
#include <string>
#include <vector>
namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;

namespace prepared_tag {
constexpr auto show_highscores  = "select_highscores"_zv;
constexpr auto update_highscore = "update_highscore"_zv;
}  // namespace prepared_tag

namespace prepared_query {
constexpr auto show_highscores =
    R"(SELECT name, score, playtime FROM retired_players ORDER BY score DESC, playtime ASC, name ASC;)"_zv;

constexpr auto update_highscore =
    R"(INSERT INTO retired_players (name, score, playtime) VALUES ($1, $2, $3) ON CONFLICT (name) DO UPDATE SET score=$2, playtime=$3;)"_zv;
}  // namespace prepared_query

/*
*  ##############    AUTHORS   #######################
*/

void HighscoreRepositoryImpl::Prepare() {
    write_connection_.prepare(prepared_tag::update_highscore, prepared_query::update_highscore);
    read_connection_.prepare(prepared_tag::show_highscores, prepared_query::show_highscores);
}

void HighscoreRepositoryImpl::WriteResult(game::GameResult&& result) {
    pqxx::work work{write_connection_};

    work.exec_prepared(prepared_tag::update_highscore, result.player, result.score, result.playTime_s);
    work.commit();
}

std::vector<game::GameResult> HighscoreRepositoryImpl::LoadResults(uint32_t count, uint32_t offset,
                                                                   std::string_view name) {
    std::vector<game::GameResult> result;
    pqxx::read_transaction        r(read_connection_);

    for (auto& [name, score, time] : r.query<std::string, int, double>(
             "SELECT name, score, playtime FROM retired_players ORDER BY score DESC, playtime ASC, name ASC;")) {
        result.emplace_back(name, score, time);
    }
    return result;
}

/*
*
* _____________ DATABASE _____________
*
*/

Database::Database(pqxx::connection write, pqxx::connection read)
    : write_connection_{std::move(write)}, read_connection_{std::move(read)} {
    pqxx::work work{write_connection_};
    // create highscores table

    work.exec(
        R"( CREATE TABLE IF NOT EXISTS retired_players ( id SERIAL, name varchar(50) PRIMARY KEY, score INTEGER DEFAULT 0, playTime real NOT NULL );)"_zv);
    work.commit();

    highscores_.Prepare();
}

}  // namespace postgres