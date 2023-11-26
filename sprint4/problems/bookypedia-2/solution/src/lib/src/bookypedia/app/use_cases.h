#pragma once
#include <bookypedia/domain/author.h>
#include <bookypedia/domain/book.h>
#include <bookypedia/domain/domain_fwd.h>
#include <string>
#include <vector>
namespace app {
class UseCases {
public:
    virtual void                        AddAuthor(const std::string& name)             = 0;
    virtual void                        DeleteAuthor(const domain::AuthorId& authorId) = 0;
    virtual void                        EditAuthor(const domain::Author& edited)       = 0;
    virtual std::vector<domain::Author> GetAuthors(const std::string& name) const      = 0;

    virtual void                      AddBook(const domain::Book& name)                 = 0;
    virtual std::vector<domain::Book> GetBooks(const std::string& name) const           = 0;
    virtual std::vector<domain::Book> GetAuthorBooks(const std::string& authorId) const = 0;
    virtual void                      DeleteBook(const domain::BookId& bookId)          = 0;

protected:
    ~UseCases() = default;
};
}  // namespace app
