#pragma once
#include "book.h"
#include "dbproxy.h"

using namespace model;
namespace command {
struct CreateBookTableCommand {};
struct DropBookTableCommand {};
struct AddBookCommand {
    AddBookCommand(const Book& val) : data(val) {}
    Book data;
};

}  // namespace command

namespace query {

struct ShowBooksQuery {};

}  // namespace query