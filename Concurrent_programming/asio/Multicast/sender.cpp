#include <iostream>
#include <sstream>
#include <string>
#include <boost/asio.hpp>

constexpr short multicat_port = 30001;
constexpr int max_message_count = 15;

class Sender
{
public:
    Sender(boost::asio::io_context& io_context,const boost::asio::ip::address& multicat_address)
    :_endpoint(multicat_address,multicat_port)
    ,_socket(io_context,_endpoint.protocol())
    ,_timer(io_context)
    ,_message_count(0)
    {
        do_send();
    }

private:
    void do_send()
    {
        std::ostringstream os;
        os << "Message " << _message_count ++;
        _message = os.str();

        // 异步发送
        _socket.async_send_to(
            boost::asio::buffer(_message),_endpoint,
            [this](boost::system::error_code ec,std::size_t  /*length*/ )
            {
                if(!ec && _message_count < max_message_count)
                {
                    do_timeout();
                }
            }
        );
    }

    void do_timeout()
    {
        _timer.expires_after(std::chrono::seconds(1));
        _timer.async_wait(
            [this](boost::system::error_code ec)
            {
                if(!ec)
                {
                    do_send();
                }
            }
        );
    }

private:
    boost::asio::ip::udp::endpoint _endpoint;
    boost::asio::ip::udp::socket _socket;
    boost::asio::steady_timer _timer;
    int _message_count;
    std::string _message;
};

int main(int argc,char* argv[])
{ 
    try
    {
        if(argc != 2)
        {
            std::cerr << "Usage: sender <multicast_address>\n";
            std::cerr << "  For IPv4, try:\n";
            std::cerr << "    sender 239.255.0.1\n";
            std::cerr << "  For IPv6, try:\n";
            std::cerr << "    sender ff31::8000:1234\n";
            return 1;
        }
        
        boost::asio::io_context io_context;
<<<<<<< HEAD
        Sender s(io_context,boost::asio::ip::make_address(argv[1]));
=======
        Sender s(io_context,boost::asio::ip::make_address(arg[1]));
>>>>>>> 8738960c745bb240659f86851c3544eb403a450c
        io_context.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << '\n';
    }

    return 0;
}