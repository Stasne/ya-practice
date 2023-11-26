#include "use_cases_impl.h"

#include <bookypedia/domain/author.h>
#include <bookypedia/domain/book.h>

namespace app {
using namespace domain;

void UseCasesImpl::AddAuthor(const std::string& name) {
    authors_.Save({AuthorId::New(), name});
}

void UseCasesImpl::DeleteAuthor(const domain::AuthorId& authorId) {
    authors_.Delete(authorId);
}
void UseCasesImpl::EditAuthor(const domain::Author& edited) {
    authors_.Save(edited);
}

std::vector<domain::Author> UseCasesImpl::GetAuthors(const std::string& name) const {
    return authors_.Load(name);
}

void UseCasesImpl::AddBook(const domain::Book& book) {
    books_.Save(book);
}

std::vector<domain::Book> UseCasesImpl::GetBooks(const std::string& name) const {
    return books_.Load(name);
}

std::vector<domain::Book> UseCasesImpl::GetAuthorBooks(const std::string& authorId) const {
    return books_.Load(AuthorId::FromString(authorId));
}

void UseCasesImpl::DeleteBook(const domain::BookId& bookId) {
    books_.Delete(bookId);
}

}  // namespace app
