#pragma once
#include <string>
using namespace std::literals;

namespace ContentType {
static const std::string TEXT_HTML{"text/html"};
static const std::string TEXT_PLAIN{"text/plain"};
static const std::string APP_JSON{"application/json"};
// При необходимости внутрь ContentType можно добавить и другие типы контента
};  // namespace ContentType

namespace Endpoint {
static inline constexpr std::string_view API = "/api/"sv;
static inline constexpr std::string_view GAME = "/api/v1/game/"sv;
static inline constexpr std::string_view JOIN_GAME = "/api/v1/game/join"sv;
static inline constexpr std::string_view PLAYERS_LIST = "/api/v1/game/players"sv;
};  // namespace Endpoint