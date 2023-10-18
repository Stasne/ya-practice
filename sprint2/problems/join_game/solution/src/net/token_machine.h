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

namespace {
static std::unordered_map<std::string, std::weak_ptr<Token>> ValidTokens{};
}

static std::shared_ptr<Token> CreateToken() {
    boost::uuids::random_generator generator;
    boost::uuids::uuid tokenUUID = generator();
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const auto& byte : tokenUUID) {
        ss << std::setw(2) << static_cast<unsigned>(byte);
    }
    auto tokenString = ss.str();
    auto token = std::make_shared<Token>(tokenString);
    ValidTokens[tokenString] = token;

    return token;
}
static bool ValidateToken(std::string tokenString) {
    if (!ValidTokens.count(tokenString)) {
        for (const auto& pair : ValidTokens) {
            BOOST_LOG_TRIVIAL(info) << " " << pair.first;
        }
        return false;
    }
    if (ValidTokens.at(tokenString).expired()) {
        ValidTokens.erase(tokenString);
        return false;
    }
    return true;
}
static inline bool ValidateToken(const Token& token) {
    return ValidateToken(*token);
}
static inline void RemoveToken(std::string tokenString) {
    ValidTokens.erase(tokenString);
}
static inline void RemoveToken(const Token& token) {
    RemoveToken(*token);
}
}  // namespace token
}  // namespace security