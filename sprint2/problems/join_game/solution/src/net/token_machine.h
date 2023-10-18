#pragma once
#include <logger.h>
#include <tagged.h>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace detail {
struct TokenTag {};
}  // namespace detail

namespace security {
namespace token {
using Token = util::Tagged<std::string, detail::TokenTag>;

std::shared_ptr<Token> CreateToken();
bool ValidateToken(std::string tokenString);
bool ValidateToken(const Token& token);
void RemoveToken(std::string tokenString);
void RemoveToken(const Token& token);
}  // namespace token
}  // namespace security