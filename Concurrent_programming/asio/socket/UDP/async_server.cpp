#include <ctime>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

std::string make_daytime_string()
{
    using namespace std; // For time_t, time and ctime;
    time_t now = time(0);
    return ctime(&now);
}

class udp_server 
{
public:
    udp_server(boost::asio::io_context& io_context)
    :_socket(io_context,udp::endpoint(udp::v4,13))
    {
        start_receive();
    }

private:
    void start_receive()
    {
        _socket.async_receive_from(
            boost::asio::buffer(_recv_buffer),_remote_endpoint,
            boost::bind(&udp_server::handle_receive,this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
        )
    }

    void handle_receive(const boost::system::error_code& error,
            std::size_t   /*bytes_transferred*/)
    {
        if(!error)
        {
            boost::shared_ptr<std::string> message(
                new std::string(make_daytime_string()));

            _socket.async_send_to(boost_asio::buffer(*message)_remote_endpoint,
                boost::bind(&udp_server::handle_send,this,message,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
            
            start_receive()
        }
    }


    void handle_send(boost::shared_ptr<std::string> /*message*/,
      const boost::system::error_code& /*error*/,
      std::size_t /*bytes_transferred*/)
    {
    }    

    udp::socket _socket;
    udp::endpoint _remote_endpoint;
    boost::array<char,1> _recv_buffer;
};

int main()
{
    try
    {
        boost::asio::io_context io_context;
        udp_server server(io_context);
        io_context.run();
    }
    catch (std::exception* e)
    {
        std::cerr << e.what() <<std::endl;
    }
    return 0;
}