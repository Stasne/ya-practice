#pragma once
#include <book.h>
#include <boost/json.hpp>
#include <iostream>
using namespace model;
namespace convertion {

namespace naming {
static const std::string Payload{"payload"};
static const std::string Action{"action"};
}  // namespace naming

struct CmdObject {
    std::string         cmd;
    boost::json::object payload;
};

CmdObject ParseRequest(std::string_view jsonString) {
    boost::json::value val;
    CmdObject          result;
    try {

        val          = boost::json::parse(jsonString);
        auto obj     = val.as_object();
        auto payload = obj[naming::Payload].as_object();
        auto cmd     = std::string(obj[naming::Action].as_string());
        result       = CmdObject(cmd, payload);

    } catch (const std::exception& e) {
        // std::cerr << e.what() << '\n';
        return {};
    }
    return result;
}

}  // namespace convertion
namespace {
constexpr auto ISBN   = "ISBN";
constexpr auto Title  = "title";
constexpr auto Author = "author";
constexpr auto Year   = "year";
constexpr auto Id     = "id";
}  // namespace
namespace model {

Book tag_invoke(boost::json::value_to_tag<Book>, const boost::json::value& jv) {
    if (!jv.is_object())
        throw std::runtime_error("Expected a boost::json::object");

    boost::json::object const& obj    = jv.as_object();
    const auto                 title  = obj.at(Title).as_string().c_str();
    const auto                 author = obj.at(Author).as_string().c_str();
    const uint32_t             year   = static_cast<uint32_t>(obj.at(Year).as_int64());
    std::optional<std::string> isbn;
    auto                       jval = obj.at(ISBN);
    if (!obj.at(ISBN).is_null())
        isbn = obj.at(ISBN).as_string().c_str();

    if (obj.if_contains(Id)) {
        const uint32_t id = static_cast<uint32_t>(obj.at(Id).as_int64());
        return Book(title, author, year, isbn, id);
    }
    return Book(title, author, year, isbn);
}

void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, const Book& book) {
    boost::json::object obj;
    obj[Id]     = book.Id();
    obj[Title]  = book.Title();
    obj[Author] = book.Author();
    obj[Year]   = book.Year();
    if (book.Isbn())
        obj[ISBN] = *book.Isbn();
    else
        obj[ISBN] = nullptr;

    jv = std::move(obj);
}

}  // namespace model