#pragma once
#include <pqxx/connection>
#include <pqxx/transaction>

#include <bookypedia/app/unit_of_work.h>
#include <bookypedia/domain/author.h>
#include <bookypedia/domain/book.h>

namespace postgres {

class AuthorRepositoryImpl : public domain::IAuthorRepository {
public:
    explicit AuthorRepositoryImpl(pqxx::work& work) : work_(work) {}

    void                          Save(const domain::Author& author) override;
    domain::Author                GetAuthorById(const domain::AuthorId& id) override;
    std::optional<domain::Author> GetAuthorByName(const std::string& name) override;
    void                          EditAuthorName(const domain::AuthorId& id, const std::string& name) override;
    void                          Delete(const domain::AuthorId& id) override;
    domain::Authors               GetAllAuthors() override;

private:
    pqxx::work& work_;
};

class BookRepositoryImpl : public domain::IBookRepository {
public:
    explicit BookRepositoryImpl(pqxx::work& work) : work_(work) {}

    void          Save(const domain::Book& book) override;
    void          Edit(const domain::BookId& id, const std::string& title, int publication_year,
                       const domain::Tags& tags) override;
    void          Delete(const domain::BookId& id) override;
    domain::Books GetAllBooks() override;
    domain::Books GetBooksByAuthorId(const domain::AuthorId& id) override;

private:
    domain::Tags GetBookTags(const domain::BookId& id);

    pqxx::work& work_;
};

class UnitOfWorkImpl : public app::UnitOfWork {
public:
    explicit UnitOfWorkImpl(pqxx::connection& connection) : work_(connection), authors_(work_), books_(work_) {}

    void Commit() override { work_.commit(); }

    domain::IAuthorRepository& Authors() override { return authors_; }

    domain::IBookRepository& Books() override { return books_; }

private:
    pqxx::work           work_;
    AuthorRepositoryImpl authors_;
    BookRepositoryImpl   books_;
};

class UnitOfWorkFactoryImpl : public app::UnitOfWorkFactory {
public:
    explicit UnitOfWorkFactoryImpl(pqxx::connection& connection) : connection_(connection) {}

    app::UnitOfWorkUnit CreateUnit() override { return std::make_unique<UnitOfWorkImpl>(connection_); }

private:
    pqxx::connection& connection_;
};

class Database {
public:
    explicit Database(pqxx::connection connection);

    app::UnitOfWorkFactory& GetUnitOfWorkFactory() { return unit_factory_; }

private:
    pqxx::connection      connection_;
    UnitOfWorkFactoryImpl unit_factory_{connection_};
};

}  // namespace postgres