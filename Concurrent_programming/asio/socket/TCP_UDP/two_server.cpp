#include <ctime>
#include <string>
#include <iostream>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

std::string make_daytime_string()
{
   using namespace std;    // for time, time_t, ctime 
   time_t now = time(0);
   return ctime(&now);
}


class tcp_connection : public boost::enable_shared_from_this<tcp_connection>
{
private:
   tcp_connection(boost::asio::io_context& io_context)
   :_socket(io_context)
   { 

   }

   void handle_write()
   {

   }

   tcp::socket _socket;
   std::string _message;

public:
   typedef boost::shared_ptr<tcp_connection> pointer;

   static pointer create( boost::asio::io_context& io_context)
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
         boost::bind(&tcp_connection::handle_write,shared_from_this()));
   }
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
       tcp_connection::pointer new_connection = 
           tcp_connection::create(_acceptor.get_executor().context());

       _acceptor.async_accept(new_connection->socket(),
                boost::bind(&tcp_server::handle_accept,this,new_connection,boost::asio::placeholders::error));
   }
    
   void handle_accept(tcp_connection::pointer new_connection,const boost::system::error_code& error)
   {
        if(!error)
        {
            new_connection->start();
        }

        start_accept();
   }
private:
   tcp::acceptor _acceptor;
};

class udp_server
{
public:
    udp_server(boost::asio::io_context& io_context)
    :_socket(io_context,udp::endpoint(udp::v4(),13))
    {
        start_receive();
    }

    void start_receive()
    {
        _socket.async_receive_from(boost::asio::buffer(_recv_buffer),_remote_endpoint,
                boost::bind(&udp_server::handle_receive,this,boost::asio::placeholders::error));
    }
    
    void handle_receive(const boost::system::error_code& error)
    {
        if(!error)
        {
            boost::shared_ptr<std::string> messages(new std::string(make_daytime_string()));
            _socket.async_send_to(boost::asio::buffer(*messages),_remote_endpoint,
                                  boost::bind(&udp_server::handle_send,this,messages));

            start_receive();
        }
    }

    void handle_send(boost::shared_ptr<std::string> /*message*/)
    {

    }

private:
    udp::socket _socket;
    udp::endpoint _remote_endpoint;
    boost::array<char, 1> _recv_buffer;
};

int main()
{
    try 
    {
        boost::asio::io_context io_context;
        tcp_server server1(io_context);
        udp_server server2(io_context);

        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}


