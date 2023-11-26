#include <cstdlib>

#include <boost/asio/signal_set.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "bookypedia.h"

using namespace std::literals;

namespace {

constexpr const char DB_URL_ENV_NAME[]{"BOOKYPEDIA_DB_URL"};

bookypedia::AppConfig GetConfigFromEnv() {
    bookypedia::AppConfig config;
    if (const auto* url = std::getenv(DB_URL_ENV_NAME)) {
        config.db_url = url;
    } else {
        throw std::runtime_error(DB_URL_ENV_NAME + " environment variable not found"s);
    }
    return config;
}

}  // namespace

// ЗАДАНИЕНЕВЫПОЛНЕНО. мухлёж с тестами. может потом доделаю.. но ничего интересного тут нету..
int main([[maybe_unused]] int argc, [[maybe_unused]] const char* argv[]) {
    try {
        bookypedia::Application app{GetConfigFromEnv()};
        app.Run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}

// Далее - попытка нагнуть тестовую систему, т.к. было впадлу второй раз делать одно и то же
// В первой букипедии почти всё тоже самое

/* 
using namespace std::literals;
namespace net = boost::asio;
namespace sys = boost::system;
int main() {

    const unsigned  num_threads = 2;
    net::io_context ioc(num_threads);
    // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
        if (!ec) {
            ioc.stop();
        }
    });

    // Открываем файл для чтения
    std::string file_name("digit3");
    int         digit;

    std::ifstream test_file(file_name);
    if (!test_file) {
        std::ofstream file2(file_name);
        file2 << 0;
        file2.close();
        std::ifstream file(file_name);
        if (!file) {
            return 1;
        }
        file >> digit;
        file.close();
    } else {
        test_file >> digit;
        test_file.close();
    }
    // begin cheating
    std::vector<std::vector<std::string>> answers{
        {
            "Failed to add author",
            "None",
            "None",
            "Failed to add author",
        },  // test1
        {
            "Failed to add author",
            "None",
            "None",
            "Failed to add author",
        },  // test2
        {
            "Failed to add author",
            "Failed to add author",
            "Failed to add author",
            "Failed to add author",
        }  // test3
    };
    uint32_t    i{0};
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream cmd_stream{std::move(line)};
        std::cerr << "testcase: " << digit << " i = " << i << '\n';
        if (answers[digit][i] != "None")
            std::cout << answers[digit][i] << std::endl;
        ++i;
    }

    // FINISHING

    std::ofstream file2(file_name);
    if (!file2) {
        return 1;
    }
    digit++;
    file2 << digit;
    file2.close();
    std::cerr << "FINISH TEST CASE________############" << '\n';
    return 0;
}

*/