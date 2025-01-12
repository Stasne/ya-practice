#pragma once
#include <bookypedia/app/unit_of_work.h>
#include <bookypedia/domain/domain_fwd.h>
#include "use_cases.h"
namespace app {

class UseCasesImpl : public UseCases {
public:
    explicit UseCasesImpl(UnitOfWorkFactory& unit_factory) : unit_factory_(unit_factory) {}

    void                          AddAuthor(const std::string& name) override;
    void                          EditAuthorName(const domain::AuthorId& id, const std::string& name) override;
    void                          DeleteAuthor(const domain::AuthorId& id) override;
    domain::Authors               GetAllAuthors() override;
    domain::Author                GetAuthorById(const domain::AuthorId& id) override;
    std::optional<domain::Author> GetAuthorByName(const std::string& name) override;

    void AddBook(const domain::Author& author, const std::string& title, int year, const domain::Tags& tags) override;
    void EditBook(const domain::BookId& id, const std::string& title, int year, const domain::Tags& tags) override;
    void DeleteBook(const domain::BookId& id) override;
    domain::Books GetAllBooks() override;
    domain::Books GetBooksByAuthorId(const domain::AuthorId& id) override;

private:
    UnitOfWorkFactory& unit_factory_;
};

}  // namespace app
