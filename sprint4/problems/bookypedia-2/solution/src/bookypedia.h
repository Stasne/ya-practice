#pragma once
#include <pqxx/pqxx>

#include <bookypedia/app/use_cases_impl.h>
#include <bookypedia/postgres/postgres.h>

namespace bookypedia {

struct AppConfig {
    std::string db_url;
};

class Application {
public:
    explicit Application(const AppConfig& config);

    void Run();

private:
    postgres::Database db_;
    app::UseCasesImpl  use_cases_{db_.GetAuthors(), db_.GetBooks()};
};

}  // namespace bookypedia
