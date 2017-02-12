#pragma once
#include <boost/asio.hpp>
#include "tcp_server_connection.h"

namespace Phyre
{
namespace Networking
{
    class TCPServer : public Logging::LoggableInterface
    {

    public:
        TCPServer(boost::asio::io_service& io_service, uint16_t listen_port, const std::queue<std::string>& message_queue = std::queue<std::string>());

        void StartAccept();
        void HandleAccept(const boost::system::error_code& error);

        // Loggable overrides
        std::string log() override { return "[TCPServer]"; }

    private:
        boost::asio::ip::tcp::acceptor acceptor_;
        TCPServerConnection::pointer ptr_connection_;
        std::queue<std::string> message_queue_;
    };
}
}

