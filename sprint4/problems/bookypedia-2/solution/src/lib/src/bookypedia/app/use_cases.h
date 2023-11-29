#pragma once
#include <bookypedia/domain/author.h>
#include <bookypedia/domain/book.h>
#include <bookypedia/domain/domain_fwd.h>
#include <string>
#include <vector>
namespace app {

class UseCases {
public:
    virtual void                          AddAuthor(const std::string& name)                                  = 0;
    virtual void                          EditAuthorName(const domain::AuthorId& id, const std::string& name) = 0;
    virtual void                          DeleteAuthor(const domain::AuthorId& id)                            = 0;
    virtual domain::Authors               GetAllAuthors()                                                     = 0;
    virtual domain::Author                GetAuthorById(const domain::AuthorId& id)                           = 0;
    virtual std::optional<domain::Author> GetAuthorByName(const std::string& name)                            = 0;

    virtual void AddBook(const domain::Author& author, const std::string& title, int year,
                         const domain::Tags& tags)                                                                = 0;
    virtual void EditBook(const domain::BookId& id, const std::string& title, int year, const domain::Tags& tags) = 0;
    virtual void DeleteBook(const domain::BookId& id)                                                             = 0;
    virtual domain::Books GetAllBooks()                                                                           = 0;
    virtual domain::Books GetBooksByAuthorId(const domain::AuthorId& id)                                          = 0;

protected:
    ~UseCases() = default;
};

}  // namespace app
