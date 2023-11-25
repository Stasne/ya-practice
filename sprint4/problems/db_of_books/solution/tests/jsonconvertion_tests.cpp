#include <gtest/gtest.h>
#include <jsonconverter.h>
using namespace std::literals;

TEST(JsonToObjects, JsonToCmd) {
    auto testJson                 = "{\"action\":\"add_book\",\"payload\":{\"title\":\"The Old Man and the Sea\"}}";
    auto [cmd, jsonObjectPayload] = convertion::ParseRequest(testJson);

    EXPECT_STREQ(cmd.c_str(), "add_book");
}
TEST(JsonToObjects, BadJsonToCmd) {
    auto testJson                 = "{\"act Sea}}";
    auto [cmd, jsonObjectPayload] = convertion::ParseRequest(testJson);

    EXPECT_STREQ(cmd.c_str(), "");
}

TEST(JsonToObjects, ParseJson_to_Book) {
    auto testJson =
        "{\"action\":\"add_book\",\"payload\":{\"title\":\"The Old Man and the "
        "Sea\",\"author\":\"Hemingway\",\"year\":1952, \"ISBN\":\"5555555555555\"}}";

    auto [cmd, jsonObjectPayload] = convertion::ParseRequest(testJson);
    auto book =
        boost::json::value_to<Book>(boost::json::value(jsonObjectPayload));  //convertion::ParseBook(jsonObjectPayload);

    EXPECT_STREQ(book.Title().c_str(), "The Old Man and the Sea");
    EXPECT_STREQ(book.Author().c_str(), "Hemingway");
    EXPECT_STREQ(book.Isbn()->c_str(), "5555555555555");
    EXPECT_EQ(book.Year(), 1952);
}
