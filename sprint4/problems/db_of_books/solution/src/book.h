#pragma once
#include <optional>
#include <string>
namespace model {
class Book {
public:
    Book()              = default;
    Book(const Book& b) = default;
    Book(std::string title, std::string author, uint32_t year, std::optional<std::string> isbn, uint32_t id = 0)
        : title_(std::move(title)), author_(std::move(author)), year_(year), isbn_(std::move(isbn)), id_(id) {}

    const uint32_t             Id() const { return id_; }
    const std::string&         Title() const { return title_; }
    const std::string&         Author() const { return author_; }
    const uint32_t             Year() const { return year_; }
    std::optional<std::string> Isbn() const { return isbn_; }

private:
    uint32_t                   id_;
    std::string                title_;
    std::string                author_;
    uint32_t                   year_;
    std::optional<std::string> isbn_;
};

}  // namespace model