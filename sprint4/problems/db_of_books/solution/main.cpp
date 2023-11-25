// main.cpp
#include <book_manager.h>
#include <console_processor.h>
#include <dbproxy.h>
#include <iostream>
#include <pqxx/pqxx>
using namespace std::literals;

using pqxx::operator"" _zv;

int main(int argc, const char* argv[]) {

    DbProxy dbProxy;
    try {
        if (argc == 1) {
            std::cout << "Usage: db_example <conn-string>\n"sv;
            return EXIT_SUCCESS;
        } else if (argc != 2) {
            std::cerr << "Invalid command line\n"sv;
            return EXIT_FAILURE;
        }

        auto conn = std::make_unique<pqxx::connection>(argv[1]);

        // std::string connString("postgres://postgres:postgres@localhost:33321/ya");
        // auto        conn = std::make_unique<pqxx::connection>(connString);
        dbProxy.Connect(std::move(conn));

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    auto bookManager = std::make_shared<DbBookManager>(std::move(dbProxy));

    ConsoleProcessor processor(bookManager);
    processor();

    // try {

    //     // auto cmd = Parsecmd(json);

    //     // switch (cmd) {
    //     //     case ADD_BOOK: {

    //     //         break;
    //     //     }
    //     // }
    // } catch (const std::exception& e) {
    //     std::cerr << e.what() << std::endl;
    //     return EXIT_FAILURE;
    // }

    // constexpr auto tag_ins_movie = "ins_movie"_zv;
    // conn.prepare(tag_ins_movie, "INSERT INTO movies (title, year) VALUES ($1, $2)"_zv);
    return EXIT_SUCCESS;
}
