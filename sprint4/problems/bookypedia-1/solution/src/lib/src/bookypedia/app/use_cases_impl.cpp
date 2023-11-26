#include "use_cases_impl.h"

#include <bookypedia/domain/author.h>
#include <bookypedia/domain/book.h>

namespace app {
using namespace domain;

void UseCasesImpl::AddAuthor(const std::string& name) {
    authors_.Save({AuthorId::New(), name});
}

std::vector<domain::Author> UseCasesImpl::GetAuthors() const {
    return authors_.Load();
}

void UseCasesImpl::AddBook(const domain::Book& book) {
    books_.Save(book);
}

std::vector<domain::Book> UseCasesImpl::GetBooks() const {
    return books_.Load();
}
std::vector<domain::Book> UseCasesImpl::GetAuthorBooks(const std::string& authorId) const {
    return books_.Load(AuthorId::FromString(authorId));
}
}  // namespace app
