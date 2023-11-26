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
    Book(BookId id, std::string name) : id_(std::move(id)), name_(std::move(name)) {}

    const BookId& GetId() const noexcept { return id_; }

    const std::string& GetName() const noexcept { return name_; }

private:
    BookId      id_;
    std::string name_;
};

class IBookRepository {
public:
    virtual void Save(const Book& book) = 0;

protected:
    ~IBookRepository() = default;
};

}  // namespace domain
