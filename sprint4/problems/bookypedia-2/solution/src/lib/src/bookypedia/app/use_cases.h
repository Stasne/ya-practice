#pragma once
#include <bookypedia/domain/domain_fwd.h>
#include <string>
#include <vector>
namespace app {
class UseCases {
public:
    virtual void                        AddAuthor(const std::string& name) = 0;
    virtual std::vector<domain::Author> GetAuthors() const                 = 0;

    virtual void                      AddBook(const domain::Book& name)                 = 0;
    virtual std::vector<domain::Book> GetBooks() const                                  = 0;
    virtual std::vector<domain::Book> GetAuthorBooks(const std::string& authorId) const = 0;

protected:
    ~UseCases() = default;
};
}  // namespace app
