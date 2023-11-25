
#include <dbproxy.h>
#include <gtest/gtest.h>
using namespace model;
std::string connString("postgres://postgres:postgres@localhost:33321/ya");

class DbProxyF : public ::testing::Test {
public:
    virtual void SetUp() override {
        try {
            auto conn = std::make_unique<pqxx::connection>(connString);
            db_.Connect(std::move(conn));
        } catch (const std::exception& e) {
            std::cerr << e.what() << '\n';
        }
    }
    virtual void TearDown() override {}

    DbProxy db_;
};

TEST_F(DbProxyF, BookTable_creation) {
    // create books table
    db_.Command(command::DropBookTableCommand());
    //expect rheow cuz no table
    EXPECT_ANY_THROW(db_.Query(query::ShowBooksQuery()));
    db_.Command(command::CreateBookTableCommand());
    //expect good request with table exists
    EXPECT_NO_THROW(db_.Query(query::ShowBooksQuery()));
}

TEST_F(DbProxyF, EmptyTableExists_BookInsert) {
    db_.Command({command::AddBookCommand(Book("Book1", "Author1", 2013, "1234567890123")),
                 command::AddBookCommand(Book("Book2", "Author2", 2017, "2234567890123"))});
    auto map = db_.Query(query::ShowBooksQuery());
    EXPECT_EQ(2, map.size());
}

TEST_F(DbProxyF, Check_Addbooks_with_same_ISBN) {
    db_.Command(command::DropBookTableCommand());
    db_.Command(command::CreateBookTableCommand());
    std::string ISBN{"4234567890123"};
    EXPECT_ANY_THROW(db_.Command({command::AddBookCommand(Book("Book1", "Author1", 2013, ISBN)),
                                  command::AddBookCommand(Book("Book2", "Author2", 2017, ISBN))}));

    auto map = db_.Query(query::ShowBooksQuery());
    EXPECT_EQ(0, map.size());
}
TEST_F(DbProxyF, Check_Insert_book_with_no_ISBN) {
    db_.Command(command::CreateBookTableCommand());
    auto map  = db_.Query(query::ShowBooksQuery());
    auto size = map.size();
    EXPECT_NO_THROW(db_.Command({command::AddBookCommand(Book("Book1", "Author1", 2013, std::nullopt))}));

    map = db_.Query(query::ShowBooksQuery());
    EXPECT_EQ(size + 1, map.size());
}
TEST_F(DbProxyF, Check_Addbooks_with_HUGE_ISBN) {
    auto        map  = db_.Query(query::ShowBooksQuery());
    auto        size = map.size();
    std::string HUGE_ISBN{"42345678901232321313213123"};
    EXPECT_ANY_THROW(db_.Command({command::AddBookCommand(Book("Book1", "Author1", 2013, HUGE_ISBN))}));

    map = db_.Query(query::ShowBooksQuery());
    EXPECT_EQ(size, map.size());
}
TEST_F(DbProxyF, AddDummyUsers) {
    db_.Command(command::DropBookTableCommand());
    db_.Command(command::CreateBookTableCommand());
    db_.Command({command::AddBookCommand(Book("Book1", "Author1", 2013, "1234567890123")),
                 command::AddBookCommand(Book("Book2", "Author2", 2017, "2234567890123")),
                 command::AddBookCommand(Book("Book3", "Author3", 2019, "2234867890123")),
                 command::AddBookCommand(Book("Book4", "Most Old Author", 1817, std::nullopt))});
    EXPECT_EQ(1, 1);
}
TEST_F(DbProxyF, Check_INJECTION_SAFETY_ISBN) {
    db_.Command(command::CreateBookTableCommand());
    auto map  = db_.Query(query::ShowBooksQuery());
    auto size = map.size();
    EXPECT_ANY_THROW(db_.Command(
        {command::AddBookCommand(Book("InjectionBook_isbn", "Author1", 2013, "111'); DROP TABLE books; --"))}));
    // не сработает, пушто ISBN = 13 символов длины
    map = db_.Query(query::ShowBooksQuery());
    EXPECT_EQ(size, map.size());
}
TEST_F(DbProxyF, Check_INJECTION_SAFETY_TITLE) {
    db_.Command(command::CreateBookTableCommand());
    auto map  = db_.Query(query::ShowBooksQuery());
    auto size = map.size();
    EXPECT_NO_THROW(db_.Command({command::AddBookCommand(
        Book("book', 'author', 1000, '111'); DROP TABLE books; --", "InjectionAuthor1", 2013, "6486958564785"))}));

    map = db_.Query(query::ShowBooksQuery());
    EXPECT_EQ(size + 1, map.size());
}
TEST_F(DbProxyF, Check_INJECTION_SAFETY_AUTHOR) {
    db_.Command(command::CreateBookTableCommand());
    auto map  = db_.Query(query::ShowBooksQuery());
    auto size = map.size();
    EXPECT_NO_THROW(db_.Command({command::AddBookCommand(
        Book("InjectionBook", "author', 1000, '111'); DROP TABLE books; --", 2013, "5486958564785"))}));

    map = db_.Query(query::ShowBooksQuery());
    EXPECT_EQ(size + 1, map.size());
}

// TEST_F(DbProxyF, Check_Addbooks_with_empty_AUTHOR) {
//     std::string ISBN{"5234567890123"};
//     EXPECT_ANY_THROW(db_.Command({command::AddBookCommand(Book("Book24", "", 2013, ISBN))}));
// }
