#pragma once
#include <bookypedia/util/tagged_uuid.h>
#include <string>
#include <vector>

namespace domain {

namespace detail {
struct AuthorTag {};
}  // namespace detail

using AuthorId = util::TaggedUUID<detail::AuthorTag>;

class Author {
public:
    Author(AuthorId id, std::string name) : id_(std::move(id)), name_(std::move(name)) {}

    const AuthorId&    GetId() const noexcept { return id_; }
    const std::string& GetName() const noexcept { return name_; }

private:
    AuthorId    id_;
    std::string name_;
};

class IAuthorRepository {
public:
    virtual void                Save(const Author& author) = 0;
    virtual std::vector<Author> Load()                     = 0;

protected:
    ~IAuthorRepository() = default;
};

}  // namespace domain
