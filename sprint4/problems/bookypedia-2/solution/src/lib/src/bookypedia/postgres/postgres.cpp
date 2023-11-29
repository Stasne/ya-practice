#include "postgres.h"
#include <fmt/core.h>
#include <iostream>
#include <pqxx/pqxx>
#include <pqxx/zview.hxx>
#include <string>
#include <vector>
namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;

/*
*  ##############    AUTHORS   #######################
*/

void AuthorRepositoryImpl::Save(const domain::Author& author) {
    work_.exec_params(R"( INSERT INTO authors (id, name) VALUES ($1, $2) ON CONFLICT (id) DO UPDATE SET name=$2; )"_zv,
                      author.GetId().ToString(), author.GetName());
}

domain::Author AuthorRepositoryImpl::GetAuthorById(const domain::AuthorId& id) {
    const auto query_text = fmt::format("SELECT id, name FROM authors WHERE id = {};", work_.quote(id.ToString()));
    const auto& [_, name] = work_.query1<std::string, std::string>(query_text);
    return domain::Author{id, name};
}

std::optional<domain::Author> AuthorRepositoryImpl::GetAuthorByName(const std::string& name) {
    const auto  query_text = fmt::format("SELECT id, name FROM authors WHERE name = {};", work_.quote(name));
    const auto& data       = work_.query01<std::string, std::string>(query_text);
    if (!data) {
        return std::nullopt;
    }
    const auto& [id, author_name] = *data;
    return domain::Author{domain::AuthorId::FromString(id), author_name};
}

void AuthorRepositoryImpl::EditAuthorName(const domain::AuthorId& id, const std::string& name) {
    work_.exec_params(R"( UPDATE authors SET name = $1 WHERE id = $2 )", name, id.ToString());
}

void AuthorRepositoryImpl::Delete(const domain::AuthorId& id) {
    // delete all book tags
    work_.exec("DELETE FROM book_tags WHERE book_id IN ( SELECT id FROM books WHERE author_id =" +
               work_.quote(id.ToString()) + " );");
    // delete all books
    work_.exec("DELETE FROM books WHERE author_id = " + work_.quote(id.ToString()));
    // delete author
    work_.exec("DELETE FROM authors WHERE id = " + work_.quote(id.ToString()));

    work_.commit();
}

domain::Authors AuthorRepositoryImpl::GetAllAuthors() {
    const auto      query_text = "SELECT id, name FROM authors ORDER BY name"_zv;
    domain::Authors authors;

    for (const auto& [id, name] : work_.query<std::string, std::string>(query_text)) {
        authors.emplace_back(domain::AuthorId::FromString(id), name);
    }

    return authors;
}

/*
*
*  ##############  Book workspace #######################
*/

void BookRepositoryImpl::Save(const domain::Book& book) {
    work_.exec_params(
        R"( INSERT INTO books (id, title, author_id, publication_year) VALUES ($1, $2, $3, $4) ON CONFLICT (id) DO UPDATE SET title=$2, author_id=$3, publication_year=$4; )"_zv,
        book.GetId().ToString(), book.GetTitle(), book.GetAuthorId().ToString(), book.GetPublicationYear());

    for (const auto& tag : book.GetTags()) {
        work_.exec_params("INSERT INTO book_tags (book_id, tag) VALUES($1, $2);", book.GetId().ToString(), tag);
    }
}

void BookRepositoryImpl::Edit(const domain::BookId& id, const std::string& title, int publication_year,
                              const domain::Tags& tags) {
    work_.exec_params(R"( UPDATE books SET title = $1, publication_year = $2 WHERE id = $3 )", title, publication_year,
                      id.ToString());
    work_.exec_params(R"( DELETE FROM book_tags WHERE book_id = $1 )", id.ToString());

    for (const auto& tag : tags) {
        work_.exec_params("INSERT INTO book_tags (book_id, tag) VALUES($1, $2);", id.ToString(), tag);
    }
}

void BookRepositoryImpl::Delete(const domain::BookId& id) {
    work_.exec("DELETE FROM book_tags WHERE book_id = " + work_.quote(id.ToString()));
    work_.exec("DELETE FROM books WHERE id = " + work_.quote(id.ToString()));
    work_.commit();
}

domain::Tags BookRepositoryImpl::GetBookTags(const domain::BookId& id) {
    domain::Tags tags;

    const auto query_text =
        fmt::format(R"( SELECT tag FROM book_tags WHERE book_id = {} ORDER BY tag; )", work_.quote(id.ToString()));

    for (const auto& [tag] : work_.query<std::string>(query_text)) {
        tags.push_back(tag);
    }
    return tags;
}

domain::Books BookRepositoryImpl::GetAllBooks() {
    domain::Books books;

    const auto query_text =
        R"( SELECT books.id, books.title, books.author_id, authors.name, books.publication_year FROM books JOIN authors ON books.author_id = authors.id ORDER BY books.title;)"_zv;

    for (const auto& [id, title, author_id, author_name, publication_year] :
         work_.query<std::string, std::string, std::string, std::string, int>(query_text)) {
        auto book_id = domain::BookId::FromString(id);
        books.emplace_back(book_id, domain::AuthorId::FromString(author_id), title, publication_year,
                           GetBookTags(book_id), author_name);
    }

    return books;
}

domain::Books BookRepositoryImpl::GetBooksByAuthorId(const domain::AuthorId& id) {
    domain::Books books;
    const auto    query_text = fmt::format(
        R"( SELECT id, title, author_id, publication_year FROM books WHERE author_id = {} ORDER BY publication_year, title; )",
        work_.quote(id.ToString()));

    for (const auto& [id, title, author_id, publication_year] :
         work_.query<std::string, std::string, std::string, int>(query_text)) {
        auto book_id = domain::BookId::FromString(id);
        books.emplace_back(book_id, domain::AuthorId::FromString(author_id), title, publication_year,
                           GetBookTags(book_id));
    }

    return books;
}

/*
*
* _____________ DATABASE _____________
*
*/

Database::Database(pqxx::connection connection) : connection_{std::move(connection)} {
    pqxx::work work{connection_};
    // create authors table
    work.exec(
        R"( CREATE TABLE IF NOT EXISTS authors ( id UUID CONSTRAINT author_id_constraint PRIMARY KEY, name varchar(50) UNIQUE NOT NULL );)"_zv);

    // create books table
    work.exec(
        R"( CREATE TABLE IF NOT EXISTS books ( id UUID PRIMARY KEY, title varchar(200) NOT NULL, author_id UUID, publication_year integer NOT NULL,  CONSTRAINT  fk_authors FOREIGN KEY (author_id) REFERENCES authors(id));)"_zv);

    // create book TAGS  table
    work.exec(
        R"( CREATE TABLE IF NOT EXISTS book_tags ( book_id UUID,  tag varchar(30) NOT NULL ,  CONSTRAINT fk_books FOREIGN KEY (book_id) REFERENCES books(id));)"_zv);

    work.commit();
}

}  // namespace postgres
