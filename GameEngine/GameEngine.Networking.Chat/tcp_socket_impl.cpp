#include <boost/bind/bind.hpp>
#include "tcp_socket_impl.h"
#include "chat_client.h"

namespace engine
{
namespace networking
{

    using boost::asio::ip::tcp;

    TCPSocketImpl::TCPSocketImpl(boost::asio::io_service& io_service, OnReadCallback on_read_callback):
        socket_(tcp::socket(io_service)), buffer_(std::vector<char>()), on_read_callback_(on_read_callback)
    {
    }

    TCPSocketImpl::~TCPSocketImpl()
    {
    }

    void TCPSocketImpl::Connect(boost::asio::ip::tcp::resolver::iterator it, OnConnectCallback& callback)
    {
        socket_.async_connect(*it, callback);
    }

    void TCPSocketImpl::Close()
    {
        socket_.close();
    }

    void TCPSocketImpl::OnRead(const boost::system::error_code& ec, size_t bytes_transferred)
    {
        on_read_callback_(ec, std::string(buffer_.begin(), buffer_.end()));
        AsyncRead();
    }

    void TCPSocketImpl::AsyncRead()
    {
        socket_.async_read_some(boost::asio::buffer(buffer_),
                                boost::bind(&TCPSocketImpl::OnRead,
                                            this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
    }

    void TCPSocketImpl::Write(std::string data)
    {
        write(socket_, boost::asio::buffer(data));		
        AsyncRead();
    }

}
}
