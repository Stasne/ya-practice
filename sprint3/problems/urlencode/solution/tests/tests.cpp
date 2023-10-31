#include <gtest/gtest.h>

#include "../src/urlencode.h"

using namespace std::literals;

TEST(UrlEncodeTestSuite, OrdinaryCharsAreNotEncoded) {
    EXPECT_EQ(UrlEncode(""sv), ""s);
    EXPECT_EQ(UrlEncode("hello"), "hello");
    EXPECT_EQ(UrlEncode("Hello/World*!"), "Hello%2fWorld%2a%21");
    EXPECT_EQ(UrlEncode("Hello World!"), "Hello+World%21");
    EXPECT_EQ(UrlEncode("!+/"), "%21%2b%2f");
    EXPECT_EQ(UrlEncode("Hello\n\tWorld*!"), "Hello%0a%09World%2a%21");

    // Тест 4: Декодирование строки с некорректным процентно-кодированным символом
    // EXPECT_THROW(UrlEncode("Hello%2XWorld"sv), std::invalid_argument);
}

/* Напишите остальные тесты самостоятельно */
