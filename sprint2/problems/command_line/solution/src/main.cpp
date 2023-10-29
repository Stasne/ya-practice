#include <json_loader.h>
#include <logger.h>
#include <logger_request_handler.h>
#include <magic_defs.h>
#include <request_handler.h>
#include <sdk.h>
#include <ticker.h>
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <optional>
#include <thread>
#include <vector>

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

struct Args {
    boost::optional<uint32_t> tick_period;
    std::string config_file;
    std::string www_root_path;
    bool random_spawn{false};
};
[[nodiscard]] std::optional<Args> ParseCommandLine(int argc, const char* const argv[]) {
    namespace po = boost::program_options;

    po::options_description desc{"All options"s};

    Args args;
    desc.add_options()("help,h", "Show help")("tick-period,t", po::value(&args.tick_period)->value_name("tick-period"s),
                                              "Game tick period in milliseconds")(
        "config-file,c", po::value(&args.config_file)->value_name("config-file"s), "Config file path")(
        "www-root,w", po::value(&args.www_root_path)->value_name("www-root"s), "www static files folder")(
        "randomize-spawn-points", po::bool_switch(&args.random_spawn), "Spawn dogs randomly on a map");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << '\n' << desc;
        return std::nullopt;
    }

    if (!vm.count("config-file")) {
        throw std::runtime_error("Config file have not been specified.");
    }
    if (!vm.count("www-root")) {
        throw std::runtime_error("Static files folder path is not specified.");
    }

    return args;
}

int main(int argc, const char* argv[]) {
    Logger::init("GameServerLog");

    try {
        auto args = ParseCommandLine(argc, argv);
        if (!args) {
            return EXIT_SUCCESS;
        }

        // 1. Загружаем карту из файла и построить модель игры
        model::Game game = json_loader::LoadGame(args->config_file);
        game.SetRandomSpawnEnabled(args->random_spawn);
        files::FileServer fileserver(args->www_root_path);
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
        auto api_strand = net::make_strand(ioc);
        std::shared_ptr<Ticker> ticker;
        if (args->tick_period) {
            ticker = std::make_shared<Ticker>(
                api_strand, 50ms, [&game](std::chrono::milliseconds delta) { game.TickTime(delta.count()); });
            ticker->Start();
        }

        http_handler::RequestHandler handler(game, fileserver, !args->tick_period.has_value());  //, api_strand);

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
        Logger::Log(custom_data, "server started"sv);

        // 6. Запускаем обработку асинхронных операций
        RunWorkers(std::max(1u, num_threads), [&ioc] { ioc.run(); });
        boost::json::value finish_data{{"code", EXIT_SUCCESS}};
        Logger::Log(finish_data, "server exited"sv);

    } catch (const std::exception& ex) {
        boost::json::value exception_data{{"code", EXIT_FAILURE}, {"exception", ex.what()}};
        Logger::Log(exception_data, "server exited"sv);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
