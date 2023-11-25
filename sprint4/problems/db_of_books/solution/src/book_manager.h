#pragma once
#include <book.h>
#include <vector>
#include "dbproxy.h"

using BooksList = std::vector<model::Book>;
class IBookManager {
public:
    // virtual void     AddBook(const Book&)              = 0;
    virtual ~IBookManager()                                     = default;
    virtual void      AddBooks(const std::vector<model::Book>&) = 0;
    virtual BooksList ShowBooks() const                         = 0;
};

class DbBookManager : public IBookManager {
public:
    virtual ~DbBookManager() = default;
    DbBookManager(DbProxy&& proxy) : db_(std::move(proxy)) {}

    void AddBooks(const std::vector<model::Book>& books) {
        std::vector<command::AddBookCommand> cmds;
        for (const auto& book : books)
            cmds.emplace_back(book);

        db_.Command(cmds);
    }

    BooksList ShowBooks() const {
        BooksList result;
        auto      tuples = db_.Query(query::ShowBooksQuery());
        for (const auto& [id, title, author, year, isbn] : tuples)
            result.emplace_back(title, author, year, isbn, id);
        return result;
    };

private:
    mutable DbProxy db_;
};