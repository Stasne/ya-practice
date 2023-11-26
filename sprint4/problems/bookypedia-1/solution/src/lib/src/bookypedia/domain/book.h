#pragma once
#include <string>

#include <bookypedia/util/tagged_uuid.h>

namespace domain {
namespace detail {
struct BookTag {};
}  // namespace detail

using BookId = util::TaggedUUID<detail::BookTag>;

class Book {
public:
    Book(BookId id, std::string title, uint32_t year, const AuthorId a_id)
        : id_(std::move(id)), title_(std::move(title)), year_(year), a_id_(a_id) {}

    const std::string& GetTitle() const noexcept { return title_; }
    const uint32_t     GetYear() const noexcept { return year_; }
    const BookId&      GetId() const noexcept { return id_; }
    const AuthorId&    GetAuthorId() const noexcept { return a_id_; }

private:
    BookId         id_;
    std::string    title_;
    uint32_t       year_;
    const AuthorId a_id_;
};

class IBookRepository {
public:
    virtual void              Save(const Book& book)  = 0;
    virtual std::vector<Book> Load()                  = 0;
    virtual std::vector<Book> Load(AuthorId authotId) = 0;

protected:
    ~IBookRepository() = default;
};

}  // namespace domain
