#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

std::string make_daytime_string()
{
    using namespace std;
    time_t now = time(0);
    return ctime(&now);
}

// 将 tcp_connection 类封装成单例模式 
class tcp_connection : public boost::enable_shared_from_this<tcp_connection>
{
public:
    typedef boost::shared_ptr<tcp_connection> pointer;

    static pointer create(boost::asio::io_context& io_context)
    {
        return pointer(new tcp_connection(io_context));
    }

    tcp::socket& socket()
    {
        return _socket;
    }

    void start()
    {
        _message = make_daytime_string();

        boost::asio::async_write(_socket,boost::asio::buffer(_message),
                boost::bind(&tcp_connection::handle_write,shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
    }


private:
    tcp_connection(boost::asio::io_context& io_context)
    :_socket(io_context)
    {   }

    void handle_write(const boost::system::error_code& /*error*/,
            size_t /*bytes_transferred*/ )
    {  }

    tcp::socket _socket;
    std::string _message;
};

class tcp_server
{
public:
    tcp_server(boost::asio::io_context& io_context)
    :_acceptor(io_context,tcp::endpoint(tcp::v4(),13))
    {
        start_accept();
    }

private:
    void start_accept()
    {
        tcp_connection::pointer new_connection = tcp_connection::create(_acceptor.get_executor().context());
        _acceptor.async_accept(new_connection->socket(),
            boost::bind(&tcp_server::handle_accept,this,new_connection,
                boost::asio::placeholders::error));
    }

    void handle_accept(tcp_connection::pointer new_connection,
        const boost::system::error_code& error)
    {
        if(!error)
        {
            new_connection -> start();
        }

        start_accept();
    }


    tcp::acceptor _acceptor;
};


int main(int argc, char const *argv[])
{
    try
    {
        boost::asio::io_context io_context;
        tcp_server server(io_context);
        io_context.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}
