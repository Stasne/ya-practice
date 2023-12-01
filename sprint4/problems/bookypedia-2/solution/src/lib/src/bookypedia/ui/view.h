#pragma once
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

namespace menu {
class Menu;
}

namespace app {
class UseCases;
}

namespace ui {
namespace detail {

struct AddBookParams {
    std::string              title;
    std::string              author_id;
    std::string              author_name;
    int                      publication_year;
    std::vector<std::string> tags;
};

struct AuthorInfo {
    std::string id;
    std::string name;
};

struct BookInfo {
    std::string              id;
    std::string              title;
    int                      publication_year;
    std::string              author_name;
    std::vector<std::string> tags;
};

}  // namespace detail

class View {
public:
    View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input, std::ostream& output);

private:
    bool AddAuthor(std::istream& cmd_input) const;
    bool EditAuthor(std::istream& cmd_input) const;
    bool DeleteAuthor(std::istream& cmd_input) const;
    bool AddBook(std::istream& cmd_input) const;
    bool EditBook(std::istream& cmd_input) const;
    bool DeleteBook(std::istream& cmd_input) const;
    bool ShowAuthors() const;
    bool ShowBooks() const;
    bool ShowBook(std::istream& cmd_input) const;
    bool ShowAuthorBooks() const;

    std::optional<std::vector<std::string>> ReadTags(std::string_view message) const;

    std::optional<detail::AuthorInfo> SelectAuthor() const;
    std::optional<std::string>        ReadAuthor(std::istream& cmd_input) const;
    std::optional<detail::AuthorInfo> ReadAuthor() const;
    std::vector<detail::AuthorInfo>   GetAuthors() const;
    std::vector<detail::BookInfo>     GetAuthorBooks(const std::string& author_id) const;

    std::optional<detail::BookInfo>      ReadBookByTitle(std::istream& cmd_input) const;
    std::string                          ReadBookTitle(std::string default_title) const;
    int                                  ReadBookPublicationYear(int default_year) const;
    std::vector<std::string>             ReadBookTags(const std::vector<std::string>& default_tags) const;
    std::optional<detail::BookInfo>      SelectBook(const std::vector<detail::BookInfo>& books) const;
    std::vector<detail::BookInfo>        GetBooks() const;
    std::optional<detail::AddBookParams> GetBookParams(std::istream& cmd_input) const;

    std::string    ReadNewAuthorName() const;
    menu::Menu&    menu_;
    app::UseCases& use_cases_;
    std::istream&  input_;
    std::ostream&  output_;
};

}  // namespace ui