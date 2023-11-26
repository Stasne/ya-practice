#include "dbproxy.h"
#include <iostream>

namespace prepared_tag {
constexpr auto show_books         = "select_books"_zv;
constexpr auto add_book           = "insert_book"_zv;
constexpr auto create_books_table = "create_book_table"_zv;
constexpr auto drop_books_table   = "drop_book_table"_zv;
}  // namespace prepared_tag

namespace prepared_query {
constexpr auto create_books_table =
    "CREATE TABLE IF NOT EXISTS books (id SERIAL PRIMARY KEY, title varchar(200) NOT NULL, author varchar(200) NOT NULL, year integer NOT NULL, ISBN varchar(13) NULL UNIQUE);"_zv;
constexpr auto insert_book = "INSERT INTO books (title, author, year, isbn) VALUES ($1, $2, $3, $4)"_zv;
constexpr auto drop_table  = "DROP TABLE IF EXISTS books;"_zv;
}  // namespace prepared_query

void DbProxy::PrepareQueries() {
    connection_->prepare(prepared_tag::create_books_table, prepared_query::create_books_table);

    Command(command::CreateBookTableCommand());
    connection_->prepare(prepared_tag::add_book, prepared_query::insert_book);
    connection_->prepare(prepared_tag::drop_books_table, prepared_query::drop_table);
}

ExecutionResult DbProxy::Command(const command::DropBookTableCommand&) {
    pqxx::work w(*connection_);
    w.exec_prepared(prepared_tag::drop_books_table);
    w.commit();
}
ExecutionResult DbProxy::Command(const command::CreateBookTableCommand&) {
    pqxx::work w(*connection_);
    w.exec_prepared(prepared_tag::create_books_table);
    w.commit();
}

ExecutionResult DbProxy::Command(const std::vector<command::AddBookCommand>& cmds) {
    pqxx::work w(*connection_);
    for (const auto& cmd : cmds) {
        auto&        book = cmd.data;
        pqxx::result res =
            w.exec_prepared(prepared_tag::add_book, book.Title(), book.Author(), book.Year(), book.Isbn());
    }
    w.commit();
};

BooksQueryResult DbProxy::Query(const query::ShowBooksQuery&) {
    pqxx::read_transaction r(*connection_);
    BooksQueryResult       result;
    // Выполняем запрос и итерируемся по строкам ответа
    for (auto [id, title, author, year, isbn] : r.query<int, std::string, std::string, int, std::optional<std::string>>(
             "SELECT * FROM books ORDER BY year DESC,title,author,ISBN ")) {
        result.push_back(std::make_tuple(id, title, author, year, isbn));
    }
    return result;
};