#include "token_machine.h"

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
bool ValidateToken(std::string tokenString) {
    if (!ValidTokens.count(tokenString))
        return false;

    if (ValidTokens.at(tokenString).expired()) {
        ValidTokens.erase(tokenString);
        return false;
    }
    return true;
}
bool ValidateToken(const Token& token) {
    return ValidateToken(*token);
}
void RemoveToken(std::string tokenString) {
    ValidTokens.erase(tokenString);
}
void RemoveToken(const Token& token) {
    RemoveToken(*token);
}
}  // namespace token
}  // namespace security