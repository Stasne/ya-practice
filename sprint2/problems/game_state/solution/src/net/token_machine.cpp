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
bool IsTokenCorrect(const Token& token) {
    // Токен должен быть 32 символа длиной.
    if ((*token).size() != 32)
        return false;

    // Проверяем, что все символы в токене являются допустимыми шестнадцатеричными символами.
    for (auto it = (*token).begin(); it != (*token).end(); ++it) {
        if (*it < '0' || (*it > '9' && *it < 'a') || *it > 'f')
            return false;
    }

    // Если все проверки прошли, то токен считается верным.
    return true;
}
bool IsTokenValid(const Token& token) {
    if (!ValidTokens.count(*token))
        return false;

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