#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include "audio.h"
#include "utils.h"
using namespace std::literals;

using boost::asio::ip::udp;
static const size_t max_buffer_size = 65000;
void StartServer(uint16_t port)
{
    Player player(ma_format_u8, 1);
    std::array<char, max_buffer_size> buffer;
    std::cout << "Waiting udp record..." << std::endl;
    try
    {
        boost::asio::io_context io_context;
        udp::socket socket(io_context, udp::endpoint(udp::v4(), port));
        udp::endpoint remote_endpoint;
        // Получаем не только данные, но и endpoint клиента
        auto size =
            socket.receive_from(boost::asio::buffer(buffer), remote_endpoint);
        size_t frames = size / player.GetFrameSize();
        std::cout << "Received " << size << " bytes, " << frames << " frames"
                  << std::endl;
        player.PlayBuffer(buffer.data(), frames, 1.5s);
        std::cout << "Playing done" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

void StartClient(uint16_t port)
{
    std::cout << "Press Enter to record message..." << std::endl;
    std::string str;
    std::getline(std::cin, str);

    Recorder recorder(ma_format_u8, 1);
    auto rec_result =
        recorder.Record(max_buffer_size / recorder.GetFrameSize(), 1.5s);
    std::cout << "Recording done" << std::endl;
    std::cout << "recorded: " << rec_result.data.size() << " bytes, "
              << rec_result.frames << " frames " << std::endl;
    //send to server
    try
    {
        boost::asio::io_context io_context;
        udp::socket socket(io_context, udp::v4());
        boost::system::error_code ec;
        auto endpoint =
            udp::endpoint(boost::asio::ip::make_address("127.0.0.1", ec), port);
        auto sentBytes = socket.send_to(
            boost::asio::buffer(rec_result.data.data(), rec_result.data.size()),
            endpoint);

        std::cout << "Record sent (" << sentBytes << " bytes), thank you, bye"
                  << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cout << "Usage: 2 arguments required:\n\t";
        std::cout << argv[0];
        std::cout << " [client/server] {udp_port}\n";
        return 1;
    }
    auto appType = argv[1];
    auto portNum = std::atoi(argv[2]);

    if (!utils::portValid(portNum))
    {
        std::cerr << "Port number is not valid: " << portNum << std::endl;
        return 2;
    }

    if (boost::iequals(appType, "client"))
    {
        std::cout << "Transmitter mode: " << std::endl;
        StartClient(portNum);
    }
    else
    {
        std::cout << "Receiver mode: " << std::endl;
        StartServer(portNum);
    }

    return 0;
}
