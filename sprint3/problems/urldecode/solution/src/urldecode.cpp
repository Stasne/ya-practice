#include "urldecode.h"

#include <cctype>
#include <charconv>
#include <sstream>
#include <stdexcept>
#include <string>

std::string UrlDecode(std::string_view str) {
    std::ostringstream decoded;

    for (size_t i = 0; i < str.size(); ++i) {
        char c = str[i];
        if (c == '%') {
            if (i + 2 >= str.size() || !std::isxdigit(str[i + 1]) || !std::isxdigit(str[i + 2])) {
                throw std::invalid_argument("Invalid percent encoding");
            }
            int hexValue;
            std::string hexStr = std::string(str.substr(i + 1, 2));
            std::istringstream hexStream(hexStr);
            hexStream >> std::hex >> hexValue;
            decoded.put(static_cast<char>(hexValue));
            i += 2;
        } else if (c == '+') {
            decoded.put(' ');
        } else {
            decoded.put(c);
        }
    }

    return decoded.str();
}