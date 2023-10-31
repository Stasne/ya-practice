#include "htmldecode.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();  // Handles case where 'to' is a substring of 'from'
    }
    return str;
}
std::string HtmlDecode(std::string_view str) {
    std::string decoded = std::string(str);
    // Словарь мнемоник и их соответствующих символов
    std::unordered_map<std::string, std::string> mnemonics = {
        {"&lt;", "<"}, {"&gt;", ">"}, {"&amp;", "&"}, {"&apos;", "\'"}, {"&quot;", "\""},
    };
    std::unordered_map<std::string, std::string> mnemonicsShort = {
        {"&lt", "<"}, {"&gt", ">"}, {"&amp", "&"}, {"&apos", "\'"}, {"&quot", "\""},
    };
    for (const auto& m : mnemonics) {
        auto upperKey = m.first;
        std::transform(upperKey.begin(), upperKey.end(), upperKey.begin(),
                       [](unsigned char c) { return std::toupper(c); });
        decoded = ReplaceAll(decoded, m.first, m.second);
        decoded = ReplaceAll(decoded, upperKey, m.second);
    }

    for (const auto& m : mnemonicsShort) {
        auto upperKey = m.first;
        std::transform(upperKey.begin(), upperKey.end(), upperKey.begin(),
                       [](unsigned char c) { return std::toupper(c); });
        decoded = ReplaceAll(decoded, m.first, m.second);
        decoded = ReplaceAll(decoded, upperKey, m.second);
    }

    return decoded;
}