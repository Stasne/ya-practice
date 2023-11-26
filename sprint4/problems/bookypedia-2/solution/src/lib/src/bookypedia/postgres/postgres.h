#pragma once
#include <pqxx/connection>
#include <pqxx/transaction>

#include <bookypedia/domain/author.h>
#include <bookypedia/domain/book.h>

namespace postgres {

class AuthorRepositoryImpl : public domain::IAuthorRepository {
public:
    explicit AuthorRepositoryImpl(pqxx::connection& connection);

    void                        Save(const domain::Author& author) override;
    std::vector<domain::Author> Load(const std::string& name) override;
    void                        Delete(const domain::AuthorId& authorId) override;

    void Prepare();

private:
    pqxx::connection& connection_;
};

class BookRepositoryImpl : public domain::IBookRepository {
public:
    explicit BookRepositoryImpl(pqxx::connection& connection);

    void                      Save(const domain::Book& book) override;
    std::vector<domain::Book> Load(const std::string& bookPrefix) override;
    std::vector<domain::Book> Load(domain::AuthorId authotId) override;
    void                      Delete(const domain::BookId& bookId) override;

    void Prepare();

private:
    pqxx::connection& connection_;
};

class Database {
public:
    explicit Database(pqxx::connection connection);

    AuthorRepositoryImpl& GetAuthors() & { return authors_; }
    BookRepositoryImpl&   GetBooks() & { return books_; }

private:
    pqxx::connection     connection_;
    AuthorRepositoryImpl authors_{connection_};
    BookRepositoryImpl   books_{connection_};
};

}  // namespace postgres