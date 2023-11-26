#include "postgres.h"
#include <iostream>
#include <pqxx/pqxx>
#include <pqxx/zview.hxx>
#include <string>
#include <vector>
namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;

namespace prepared_tag {
constexpr auto show_books        = "select_books"_zv;
constexpr auto show_author_books = "select_author_books"_zv;
constexpr auto add_book          = "insert_book"_zv;
constexpr auto show_authors      = "select_authors"_zv;
constexpr auto add_author        = "insert_author"_zv;
}  // namespace prepared_tag

namespace prepared_query {
constexpr auto insert_book =
    R"(INSERT INTO books (id, title, author_id, publication_year) VALUES ($1, $2, $3, $4) ON CONFLICT (id) DO UPDATE SET author_id=$3, publication_year=$4;)"_zv;

constexpr auto select_books = R"(SELECT * FROM books ORDER BY publication_year DESC, title;)"_zv;
constexpr auto select_books_by_aid =
    R"(SELECT id, title, author_id, publication_year FROM books WHERE author_id=$1 ORDER BY publication_year, title;)"_zv;
constexpr auto insert_author =
    R"(INSERT INTO authors (id, name) VALUES ($1, $2) ON CONFLICT (id) DO UPDATE SET name=$2;)"_zv;

constexpr auto select_authors     = R"(SELECT * FROM authors ORDER BY name;)"_zv;
constexpr auto drop_books_table   = R"(DROP TABLE IF EXISTS books;)"_zv;
constexpr auto drop_authors_table = R"(DROP TABLE IF EXISTS authors;)"_zv;
}  // namespace prepared_query

/*
*  ##############    AUTHORS   #######################
*/
AuthorRepositoryImpl::AuthorRepositoryImpl(pqxx::connection& connection) : connection_{connection} {}

void AuthorRepositoryImpl::Prepare() {
    connection_.prepare(prepared_tag::add_author, prepared_query::insert_author);
    connection_.prepare(prepared_tag::show_authors, prepared_query::select_authors);
}

void AuthorRepositoryImpl::Save(const domain::Author& author) {
    // Пока каждое обращение к репозиторию выполняется внутри отдельной транзакции
    // В будущих уроках вы узнаете про паттерн Unit of Work, при помощи которого сможете несколько
    // запросов выполнить в рамках одной транзакции.
    // Вы также может самостоятельно почитать информацию про этот паттерн и применить его здесь.
    pqxx::work work{connection_};
    work.exec_prepared(prepared_tag::add_author, author.GetId().ToString(),
                       author.GetName().empty() ? nullptr : author.GetName());
    work.commit();
}

std::vector<domain::Author> AuthorRepositoryImpl::Load() {
    std::vector<domain::Author> result;
    pqxx::read_transaction      r(connection_);

    for (auto& [id, name] : r.query<std::string, std::string>("SELECT id, name FROM authors ORDER BY name")) {
        result.emplace_back(domain::AuthorId::FromString(id), name);
    }
    return result;
}

/*
*
*  ##############  Book workspace #######################
*/
BookRepositoryImpl::BookRepositoryImpl(pqxx::connection& connection) : connection_{connection} {}

void BookRepositoryImpl::Prepare() {
    connection_.prepare(prepared_tag::add_book, prepared_query::insert_book);
    connection_.prepare(prepared_tag::show_books, prepared_query::select_books);
    connection_.prepare(prepared_tag::show_author_books, prepared_query::select_books_by_aid);
}

void BookRepositoryImpl::Save(const domain::Book& book) {

    pqxx::work work{connection_};
    work.exec_prepared(prepared_tag::add_book, book.GetId().ToString(), book.GetTitle(), book.GetAuthorId().ToString(),
                       book.GetYear());
    work.commit();
}

std::vector<domain::Book> BookRepositoryImpl::Load() {
    std::vector<domain::Book> result;
    pqxx::read_transaction    r(connection_);
    for (auto& [id, title, authorId, year] : r.query<std::string, std::string, std::string, int>(
             "SELECT id, title, author_id, publication_year FROM books ORDER BY title")) {
        result.emplace_back(domain::BookId::FromString(id), title, year, domain::AuthorId::FromString(authorId));
    }
    return result;
}

std::vector<domain::Book> BookRepositoryImpl::Load(domain::AuthorId aId) {
    std::vector<domain::Book> result;
    pqxx::read_transaction    r(connection_);

    // TODO"
    /* Пытался так, но не вышло.. можно ли как-то таким образом сделать?
    r.query<std::string, std::string, std::string, int>(
    "SELECT id, title, author, year FROM books ORDER BY title WHERE author=$1"), aId.ToString())
                                                                               ^         
    Жалуется на то, что я пытаюсь после запятой что-то выдумать..
    */
    auto res = r.exec_prepared(prepared_tag::show_author_books, aId.ToString());

    for (const auto& row : res) {
        // TODO:
        // а тут тоже только так? пол часа изголялся - красиво не смог (
        // Генерировать строку, где подставить 'WHERE author=___' не круто
        auto id     = row[0].as<std::string>();
        auto title  = row[1].as<std::string>();
        auto author = row[2].as<std::string>();
        auto year   = row[3].as<int>();
        result.emplace_back(domain::BookId::FromString(id), title, year, domain::AuthorId::FromString(author));
    }

    return result;
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
        R"( CREATE TABLE IF NOT EXISTS authors ( id UUID CONSTRAINT author_id_constraint PRIMARY KEY, name varchar(100) UNIQUE NOT NULL );)"_zv);

    // create books table
    work.exec(
        R"( CREATE TABLE IF NOT EXISTS books ( id UUID CONSTRAINT book_id_constraint PRIMARY KEY, title varchar(200) NOT NULL, author_id UUID REFERENCES authors(id) NOT NULL, publication_year integer NOT NULL);)"_zv);

    work.commit();
    authors_.Prepare();
    books_.Prepare();
}

}  // namespace postgres