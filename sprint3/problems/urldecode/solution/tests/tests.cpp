#define BOOST_TEST_MODULE urlencode tests
#include <boost/test/unit_test.hpp>

#include "../src/urldecode.h"

BOOST_AUTO_TEST_CASE(UrlDecode_tests) {
    using namespace std::literals;

    // Напишите остальные тесты для функции UrlDecode самостоятельно
    // Тест 1: Декодирование строки без процентно-кодированных символов
    BOOST_TEST(UrlDecode(""sv) == ""s);

    // Тест 2: Декодирование строки с пробелами и специальными символами
    BOOST_TEST(UrlDecode("Hello%20World%21"sv) == "Hello World!"s);

    // Тест 3: Декодирование строки с процентно-кодированными символами
    BOOST_TEST(UrlDecode("%21%2B%2F"sv) == "!+/"s);

    // Тест 4: Декодирование строки с некорректным процентно-кодированным символом
    BOOST_CHECK_THROW(UrlDecode("Hello%2XWorld"sv), std::invalid_argument);
}