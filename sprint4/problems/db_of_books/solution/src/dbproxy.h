#pragma once
#include <pqxx/pqxx>
#include <tuple>
#include "book_requests.h"
// libpqxx использует zero-terminated символьные литералы вроде "abc"_zv;
using pqxx::operator"" _zv;

using BookTable_row    = std::tuple<int, std::string, std::string, int, std::optional<std::string>>;
using BooksQueryResult = std::vector<BookTable_row>;
using ExecutionResult  = void;

class DbProxy {
public:
    void Connect(std::unique_ptr<pqxx::connection>&& connection) {
        connection_ = std::move(connection);
        PrepareQueries();
    }

    ExecutionResult Command(const command::CreateBookTableCommand&);
    ExecutionResult Command(const command::DropBookTableCommand&);
    ExecutionResult Command(const std::vector<command::AddBookCommand>&);

    BooksQueryResult Query(const query::ShowBooksQuery&);

private:
    void PrepareQueries();

private:
    std::unique_ptr<pqxx::connection> connection_;
};