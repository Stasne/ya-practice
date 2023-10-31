#include <catch2/catch_test_macros.hpp>

#include "../src/htmldecode.h"

using namespace std::literals;

TEST_CASE("Text without mnemonics", "[HtmlDecode]") {
    // Строка без HTML-мнемоник.
    CHECK(HtmlDecode("hello"sv) == "hello"s);
    // Строка с HTML-мнемониками.
    CHECK(HtmlDecode("hell&AMP;o"sv) == "hell&o"s);
    // Пустая строка.
    CHECK(HtmlDecode(""sv) == ""s);
    // Строка с HTML-мнемониками, записанными в верхнем регистре.
    CHECK(HtmlDecode("h&APOS;ell&AMP;o"sv) == "h'ell&o"s);
    // Строка с HTML-мнемониками, записанными в смешанном регистре.
    CHECK(HtmlDecode("h&ApoS;ell"sv) == "h&ApoS;ell"s);
    // Строка с HTML-мнемониками в начале, конце и середине.
    CHECK(HtmlDecode("&APOS;ell&amp;o&AMP"sv) == "'ell&o&"s);
    // Строка с недописанными HTML-мнемониками.
    CHECK(HtmlDecode("hell&AMo"sv) == "hell&AMo"s);
    // Строка с HTML-мнемониками, заканчивающимися и не заканчивающимися на точку с запятой.
    CHECK(HtmlDecode("h&APOS;ell&AMPo"sv) == "h'ell&o"s);
}

// Напишите недостающие тесты самостоятельно
