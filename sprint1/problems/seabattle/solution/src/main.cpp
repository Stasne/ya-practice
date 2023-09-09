#ifdef WIN32
#include <sdkddkver.h>
#endif

#include "seabattle.h"

#include <atomic>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <thread>

namespace net = boost::asio;
using net::ip::tcp;
using namespace std::literals;

void PrintFieldPair(const SeabattleField& left, const SeabattleField& right)
{
    auto left_pad = "  "s;
    auto delimeter = "    "s;
    std::cout << left_pad;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << delimeter;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << std::endl;
    for (size_t i = 0; i < SeabattleField::field_size; ++i)
    {
        std::cout << left_pad;
        left.PrintLine(std::cout, i);
        std::cout << delimeter;
        right.PrintLine(std::cout, i);
        std::cout << std::endl;
    }
    std::cout << left_pad;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << delimeter;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << std::endl;
}

template <size_t sz>
static std::optional<std::string> ReadExact(tcp::socket& socket)
{
    boost::array<char, sz> buf;
    boost::system::error_code ec;

    net::read(socket, net::buffer(buf), net::transfer_exactly(sz), ec);

    if (ec)
    {
        return std::nullopt;
    }

    return {{buf.data(), sz}};
}

static bool WriteExact(tcp::socket& socket, std::string_view data)
{
    boost::system::error_code ec;

    net::write(socket, net::buffer(data), net::transfer_exactly(data.size()), ec);

    return !ec;
}

class SeabattleAgent
{
   public:
    SeabattleAgent(const SeabattleField& field) : my_field_(field) {}

    void StartGame(tcp::socket& socket, bool my_initiative)
    {
        while (!IsGameEnded())
        {
            PrintFields();
            if (my_initiative)
            {
                // запросить ход с консоли
                std::cout << "Сделайте ход (например: А5)" << std::endl;
                std::string move;
                std::cin >> move;
                std::cout << "Мой ход: " << move << std::endl;
                // auto smove = MoveToString({move});  //?
                // отправить ход
                if (!WriteExact(socket, move))
                {
                    throw std::runtime_error("Не удалось отправить ход, игра прервана");
                }
                //получить результат
                auto result = ReadExact<1>(socket);
                if (!result)
                {
                    throw std::runtime_error("Не уделось получить результат хода");
                }
                auto parsedRes = ParseShotRes(result.value());
                if (!parsedRes)
                    throw std::runtime_error("Не уделось разобрать результат хода");

                if (parsedRes.value() != SeabattleField::ShotResult::MISS)
                    std::cout << "Результат хода: Не промазал ))" << std::endl;
                else
                    std::cout << "Результат хода: МИМО ))" << std::endl;
                auto shotResult = static_cast<SeabattleField::ShotResult>(atoi(result.value().c_str()));
                auto parsedMove = ParseMove(move);
                if (!parsedMove)
                {
                    //?
                }
                switch (shotResult)
                {
                    case SeabattleField::ShotResult::MISS:
                        other_field_.MarkMiss(parsedMove.value().second, parsedMove.value().first);
                        my_initiative = false;
                        break;
                    case SeabattleField::ShotResult::HIT:
                        other_field_.MarkHit(parsedMove.value().second, parsedMove.value().first);
                        break;
                    case SeabattleField::ShotResult::KILL:
                        other_field_.MarkKill(parsedMove.value().second, parsedMove.value().first);
                        break;
                    default:
                        //?
                        my_initiative = false;
                        break;
                }
            }
            else
            {
                // получить ход соперника
                auto result = ReadExact<2>(socket);
                if (!result)
                {
                    throw std::runtime_error("Не уделось получить ход соперника");
                }
                std::cout << "Ход соперника: " << result.value() << std::endl;
                auto parsedMove = ParseMove(result.value());
                if (!parsedMove)
                {
                    throw std::runtime_error("Не уделось получить ход соперника");
                }
                auto shotResult = my_field_.Shoot(parsedMove.value().second, parsedMove.value().first);
                switch (shotResult)
                {
                    case SeabattleField::ShotResult::MISS:
                        my_field_.MarkMiss(parsedMove.value().second, parsedMove.value().first);
                        my_initiative = true;
                        break;
                    case SeabattleField::ShotResult::HIT:
                        my_field_.MarkHit(parsedMove.value().second, parsedMove.value().first);
                        my_initiative = false;
                        break;
                    case SeabattleField::ShotResult::KILL:
                        my_field_.MarkKill(parsedMove.value().second, parsedMove.value().first);
                        my_initiative = false;
                        break;
                    default:
                        //?
                        my_initiative = false;
                        break;
                }
                auto shotStr = ShotResString(shotResult);

                if (!WriteExact(socket, shotStr))
                {
                    throw std::runtime_error("Не удалось отправить ход, игра прервана");
                }
            }
        }
        if (my_field_.IsLoser())
            std::cout << "ПОРАЖЕНИЕ" << std::endl;
        else
            std::cout << "ПОБЕДА!" << std::endl;
        std::cout << "Игра закончена, спасибо" << std::endl;
    }

   private:
    static std::optional<std::pair<int, int>> ParseMove(const std::string_view& sv)
    {
        if (sv.size() != 2)
            return std::nullopt;

        int p1 = sv[0] - 'A', p2 = sv[1] - '1';

        if (p1 < 0 || p1 > 7)
            return std::nullopt;
        if (p2 < 0 || p2 > 7)
            return std::nullopt;

        return {{p1, p2}};
    }
    static std::optional<SeabattleField::ShotResult> ParseShotRes(const std::string_view& sv)
    {
        std::cout << " ParseShotRes(): " << sv << std::endl;
        if (sv.size() != 1)
            return std::nullopt;
        int val = sv[0] - '0';
        auto shotRes = static_cast<SeabattleField::ShotResult>(val);

        if (shotRes < SeabattleField::ShotResult::MISS || shotRes > SeabattleField::ShotResult::KILL)
            return std::nullopt;

        return {shotRes};
    }
    static std::string ShotResString(SeabattleField::ShotResult result)
    {
        std::cout << " результат хода: " << static_cast<int>(result) << std::endl;
        char buff[] = {static_cast<char>(result) + '0'};
        return {buff, 1};
    }
    static std::string MoveToString(std::pair<int, int> move)
    {
        char buff[] = {static_cast<char>(move.first) + 'A', static_cast<char>(move.second) + '1'};
        return {buff, 2};
    }

    void PrintFields() const { PrintFieldPair(my_field_, other_field_); }

    bool IsGameEnded() const { return my_field_.IsLoser() || other_field_.IsLoser(); }

    // TODO: добавьте методы по вашему желанию

   private:
    SeabattleField my_field_;
    SeabattleField other_field_;
};

void StartServer(const SeabattleField& field, unsigned short port)
{
    SeabattleAgent agent(field);

    net::io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(net::ip::make_address("127.0.0.1"), port));
    std::cout << "Waiting for connection..."sv << std::endl;
    boost::system::error_code ec;
    tcp::socket socket{io_context};
    acceptor.accept(socket, ec);

    if (ec)
    {
        std::cout << "Can't accept connection"sv << std::endl;
        throw std::runtime_error("Не удалось запустить сервер");
    }
    agent.StartGame(socket, false);
};

void StartClient(const SeabattleField& field, const std::string& ip_str, unsigned short port)
{
    SeabattleAgent agent(field);
    boost::system::error_code ec;
    auto endpoint = tcp::endpoint(net::ip::make_address(ip_str, ec), port);
    if (ec)
    {
        throw std::runtime_error("Неверный формат ИП-адреса");
    }
    net::io_context io_context;
    tcp::socket socket{io_context};
    socket.connect(endpoint, ec);

    if (ec)
    {
        throw std::runtime_error("Не удалось подключиться к серверу");
    }
    agent.StartGame(socket, true);
};

int main(int argc, const char** argv)
{
    if (argc != 3 && argc != 4)
    {
        std::cout << "Usage: program <seed> [<ip>] <port>" << std::endl;
        return 1;
    }

    std::mt19937 engine(std::stoi(argv[1]));
    SeabattleField fieldL = SeabattleField::GetRandomField(engine);
    try
    {
        if (argc == 3)
        {
            StartServer(fieldL, std::stoi(argv[2]));
        }
        else if (argc == 4)
        {
            StartClient(fieldL, argv[2], std::stoi(argv[3]));
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
    catch (...)
    {
        std::cerr << "Неожиданное исключение";
        return 2;
    }
}
