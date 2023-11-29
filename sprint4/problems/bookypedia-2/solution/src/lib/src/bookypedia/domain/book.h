#pragma once
#include <bookypedia/util/tagged_uuid.h>
#include <optional>
#include <string>

namespace domain {
namespace detail {
struct BookTag {};
}  // namespace detail

using Tags   = std::vector<std::string>;
using BookId = util::TaggedUUID<detail::BookTag>;
class Book {
public:
    Book(BookId id, AuthorId author_id, std::string title, int publication_year, Tags tags,
         std::string author_name = "")
        : id_(std::move(id)),
          author_id_(std::move(author_id)),
          title_(std::move(title)),
          publication_year_(std::move(publication_year)),
          tags_(std::move(tags)) {}

    const BookId&      GetId() const noexcept { return id_; }
    const std::string& GetTitle() const noexcept { return title_; }
    const int          GetPublicationYear() const noexcept { return publication_year_; }
    const Tags&        GetTags() const noexcept { return tags_; }

    const AuthorId&    GetAuthorId() const noexcept { return author_id_; }
    const std::string& GetAuthorName() const noexcept { return author_name_; }

private:
    BookId      id_;
    AuthorId    author_id_;
    std::string title_;
    std::string author_name_;
    int         publication_year_;
    Tags        tags_;
};

using Books = std::vector<Book>;

class IBookRepository {
public:
public:
    virtual void  Save(const Book& book)                                                                   = 0;
    virtual void  Edit(const BookId& id, const std::string& title, int publication_year, const Tags& tags) = 0;
    virtual void  Delete(const BookId& id)                                                                 = 0;
    virtual Books GetAllBooks()                                                                            = 0;
    virtual Books GetBooksByAuthorId(const AuthorId& id)                                                   = 0;

protected:
    ~IBookRepository() = default;
};

}  // namespace domain
