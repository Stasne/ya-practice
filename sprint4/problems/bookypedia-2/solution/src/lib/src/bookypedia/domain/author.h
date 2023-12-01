#pragma once
#include <bookypedia/util/tagged_uuid.h>
#include <optional>
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

using Authors = std::vector<Author>;

class IAuthorRepository {
public:
    virtual void                  Save(const Author& author)                                  = 0;
    virtual void                  Delete(const AuthorId& id)                                  = 0;
    virtual Author                GetAuthorById(const AuthorId& id)                           = 0;
    virtual std::optional<Author> GetAuthorByName(const std::string& name)                    = 0;
    virtual void                  EditAuthorName(const AuthorId& id, const std::string& name) = 0;
    virtual Authors               GetAllAuthors()                                             = 0;

protected:
    ~IAuthorRepository() = default;
};

}  // namespace domain
