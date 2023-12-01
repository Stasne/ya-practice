#include "use_cases_impl.h"

#include <bookypedia/domain/author.h>
#include <bookypedia/domain/book.h>

namespace app {
using namespace domain;

void UseCasesImpl::AddAuthor(const std::string& name) {
    auto work = unit_factory_.CreateUnit();
    work->Authors().Save({AuthorId::New(), name});
    work->Commit();
}

void UseCasesImpl::EditAuthorName(const domain::AuthorId& id, const std::string& name) {
    auto work = unit_factory_.CreateUnit();
    work->Authors().EditAuthorName(id, name);
    work->Commit();
}

void UseCasesImpl::DeleteAuthor(const domain::AuthorId& id) {
    auto work = unit_factory_.CreateUnit();
    work->Authors().Delete(id);
    work->Commit();
}

std::optional<Author> UseCasesImpl::GetAuthorByName(const std::string& name) {
    auto work = unit_factory_.CreateUnit();
    return work->Authors().GetAuthorByName(name);
}

Author UseCasesImpl::GetAuthorById(const AuthorId& id) {
    auto work = unit_factory_.CreateUnit();
    return work->Authors().GetAuthorById(id);
}

void UseCasesImpl::AddBook(const domain::Author& author, const std::string& title, int year, const Tags& tags) {
    auto work = unit_factory_.CreateUnit();
    work->Authors().Save(author);
    auto book = Book(domain::BookId::New(), author.GetId(), title, year, tags);
    work->Books().Save(book);
    work->Commit();
}

void UseCasesImpl::EditBook(const domain::BookId& id, const std::string& title, int year, const domain::Tags& tags) {
    auto work = unit_factory_.CreateUnit();
    work->Books().Edit(id, title, year, tags);
    work->Commit();
}

void UseCasesImpl::DeleteBook(const domain::BookId& id) {
    auto work = unit_factory_.CreateUnit();
    work->Books().Delete(id);
    work->Commit();
}

Authors UseCasesImpl::GetAllAuthors() {
    auto work = unit_factory_.CreateUnit();
    return work->Authors().GetAllAuthors();
}

Books UseCasesImpl::GetAllBooks() {
    auto work = unit_factory_.CreateUnit();
    return work->Books().GetAllBooks();
}

Books UseCasesImpl::GetBooksByAuthorId(const AuthorId& id) {
    auto work = unit_factory_.CreateUnit();
    return work->Books().GetBooksByAuthorId(id);
}

}  // namespace app
