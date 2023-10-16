#pragma once
#include <tagged.h>
#include <iostream>
namespace detail {
struct TokenTag {};
}  // namespace detail

namespace security {
using Token = util::Tagged<std::string, detail::TokenTag>;
namespace token {
static inline bool CheckToken(const Token& token) {
    std::cout << "Checking Token '" << *token << "'\n";
    return true;
}

static inline Token CreateToken() {
    return Token("12345678");
}

}  // namespace token
}  // namespace security