#pragma once
#include <bookypedia/domain/domain_fwd.h>
#include "use_cases.h"

namespace app {

class UseCasesImpl : public UseCases {
public:
    explicit UseCasesImpl(domain::IAuthorRepository& authors, domain::IBookRepository& books)
        : authors_{authors}, books_(books) {}

    void                        AddAuthor(const std::string& name) override;
    void                        EditAuthor(const domain::Author& edited) override;
    void                        DeleteAuthor(const domain::AuthorId& bookId) override;
    std::vector<domain::Author> GetAuthors(const std::string& name) const override;

    void                      AddBook(const domain::Book& name) override;
    std::vector<domain::Book> GetBooks(const std::string& name) const override;
    std::vector<domain::Book> GetAuthorBooks(const std::string& authorId) const override;
    void                      DeleteBook(const domain::BookId& bookId) override;

private:
    domain::IAuthorRepository& authors_;
    domain::IBookRepository&   books_;
};

}  // namespace app
