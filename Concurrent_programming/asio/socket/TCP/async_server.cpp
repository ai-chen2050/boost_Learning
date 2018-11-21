#include <ctime>
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

using boost::asio::ip::tcp;

void make_daytime_string()
{
    using namespace std;    // For time,time_t, ctime
    time_t now = time(0);
    return ctime(&now);
}


/* We will use shared_ptr and enable_shared_from_this because we want to 
   keep the tcp_connection object alive as long as there is an operation that refers to it. */
class tcp_connection
: public boost::enable_shared_from_this<tcp_connection>
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
        /*  In the function start(), we call boost::asio::async_write() to 
        serve the data to the client. Note that we are using boost::asio::async_write(), 
        rather than ip::tcp::socket::async_write_some(), to ensure that the entire block of data is sent. */

        boost::asio::async_write(_socket,boost::asio::buffer(_message),
            boost::bind(&tcp::connection::handle_write, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

private:
    tcp_connection(boost::asio::io_context& io_context)
    :_socket(io_context)
    {

    }

    void handle_write(const boost::system::error_code& /* error */,
            size_t /* bytes_transferred */ )
    {

    }

    tcp::socket _socket;
    std::string _message;
};

class tcp_server
{
public:
    tcp_server(boost::asio::io_context& io_context)
    :_acceptor(io_context,tcp::endpoint(tcp::v4(),13))
    {   // 然后创建一个连接，然后异步等待
        start_accept();
    }

private:
    void start_accept()
    {
        tcp_connection::pointer new_connection = 
            tcp_connection::create(_acceptor.get_executor().context());

        // 异步接受等待
        _acceptor.async_accept(new_connection.socket(),
            boost::bind(&tcp_server::handle_accept,this,new_connection
                boost::asio::placesholder::error));
    }

    void handle_accept(tcp_connection::pointer new_connection,
            const boost::system::error_code& error)
    {
        if( !error)
        {
            new_connection->start();
        }

        start_accept();
    }

    tcp::acceptor _acceptor;
};

int mian()
{
    try
    {
        boost::asio::io_context io_context;
        tcp_server server(io_context)   ;
        io_context.run();
    }
    catch(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}