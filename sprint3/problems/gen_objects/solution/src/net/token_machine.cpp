#include "token_machine.h"
#include <regex>
using Token = util::Tagged<std::string, detail::TokenTag>;

namespace {
static std::unordered_map<std::string, std::weak_ptr<Token>> ValidTokens{};
}
namespace security {
namespace token {
std::shared_ptr<Token> CreateToken() {
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
bool IsTokenCorrect(const Token& token) {
    std::regex hexPattern("^[0-9a-fA-F]{32}$");

    // Проверка соответствия строки паттерну
    return std::regex_match(*token, hexPattern);
}
bool IsTokenValid(const Token& token) {
    return ValidTokens.count(*token);
}
bool IsAlive(const Token& token) {
    if (ValidTokens.at(*token).expired()) {
        ValidTokens.erase(*token);
        return false;
    }
    return true;
}

void RemoveToken(const Token& token) {
    ValidTokens.erase(*token);
}
}  // namespace token
}  // namespace security