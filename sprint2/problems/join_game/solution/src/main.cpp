
#include <json_loader.h>
#include <logger.h>
#include <logger_request_handler.h>
#include <magic_defs.h>
#include <request_handler.h>
#include <sdk.h>
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <iostream>
#include <thread>

using namespace std::literals;
namespace net = boost::asio;
namespace sys = boost::system;

namespace {

// Запускает функцию fn на n потоках, включая текущий
template <typename Fn>
void RunWorkers(unsigned n, const Fn& fn) {
    n = std::max(1u, n);
    std::vector<std::jthread> workers;
    workers.reserve(n - 1);
    // Запускаем n-1 рабочих потоков, выполняющих функцию fn
    while (--n) {
        workers.emplace_back(fn);
    }
    fn();
}
enum Parameters {
    BIN_NAME = 0,
    CONFIG,
    ROOT_DIR,
};
}  // namespace

int main(int argc, const char* argv[]) {
    Logger::init("GameServerLog");

    if (argc != 3) {
        std::cerr << "Usage: game_server <game-config-json> <fileserver root folder>"sv << std::endl;
        return EXIT_FAILURE;
    }
    try {
        // 1. Загружаем карту из файла и построить модель игры
        model::Game game = json_loader::LoadGame(argv[Parameters::CONFIG]);
        files::FileServer fileserver(argv[Parameters::ROOT_DIR]);
        // 2. Инициализируем io_context
        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);

        // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
            if (!ec) {
                ioc.stop();
            }
        });
        // 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
        // strand для выполнения запросов к API
        // auto api_strand = net::make_strand(ioc);
        http_handler::RequestHandler handler(game, fileserver);  //, api_strand);
        LoggingRequestHandler<http_handler::RequestHandler> handler_logged(handler);

        // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
        const auto address = net::ip::make_address(ServerParam::ADDR);
        const uint32_t uport = ServerParam::PORT;
        constexpr net::ip::port_type port = uport;
        http_server::ServeHttp(ioc, {address, port}, [&handler_logged](auto&& socket, auto&& req, auto&& send) {
            handler_logged(std::forward<decltype(socket)>(socket), std::forward<decltype(req)>(req),
                           std::forward<decltype(send)>(send));
        });
        boost::json::value custom_data{{"port", uport}, {"address", address.to_string()}};
        BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, custom_data) << "server started"sv;

        // 6. Запускаем обработку асинхронных операций
        RunWorkers(std::max(1u, num_threads), [&ioc] { ioc.run(); });
        boost::json::value finish_data{{"code", 0}};
        BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, finish_data) << "server exited"sv;

    } catch (const std::exception& ex) {
        boost::json::value exception_data{{"code", EXIT_FAILURE}, {"exception", ex.what()}};
        BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, exception_data) << "server exited"sv;
        return EXIT_FAILURE;
    }
}
