#pragma once
#include <book_manager.h>
#include <jsonconverter.h>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <memory>
class ConsoleProcessor {
public:
    ConsoleProcessor(std::shared_ptr<IBookManager> bookManager) : bm_(bookManager) {}
    void operator()() {
        std::string input;
        while (std::getline(std::cin, input)) {

            if (input.empty())
                return;
            if (!ProcessCommand(input))
                return;
        }
    }

    // private:
    bool ProcessCommand(std::string_view input) {
        auto cmdAndObject = convertion::ParseRequest(input);
        if (boost::iequals(cmdAndObject.cmd, "add_book")) {
            boost::json::object result;
            auto                book = boost::json::value_to<Book>(boost::json::value(cmdAndObject.payload));
            try {
                bm_->AddBooks({book});
                result["result"] = true;
            } catch (const std::exception& e) {
                result["result"] = false;
            }
            std::cout << result << '\n';
            return true;
        }
        if (boost::iequals(cmdAndObject.cmd, "all_books")) {
            auto               res = bm_->ShowBooks();
            boost::json::array arr;
            for (auto& book : res) {
                arr.push_back(boost::json::value_from(book));
            }
            std::cout << arr << '\n';
        }
        if (boost::iequals(cmdAndObject.cmd, "exit")) {
            return false;
        }
        return true;
    }

private:
    std::shared_ptr<IBookManager> bm_;
};
// {"action":"add_book","payload":{"title":"The Old Woman","author":"Harms","year":1939,"ISBN":"123"}}
// {"action":"all_books","payload":{}}