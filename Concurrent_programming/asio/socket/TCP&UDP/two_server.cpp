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


class tcp_connection : public enable_shared_from_this<tcp_connection>
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
   ~tcp_connection() { }

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
private:
   tcp::acceptor _acceptor;

private:

public:
   tcp_server(boost::asio::io_context& io_context)
   :_acceptor(io_context,tcp::endpoint(tcp::))
   { }
   
   ~tcp_serve() { }
};
