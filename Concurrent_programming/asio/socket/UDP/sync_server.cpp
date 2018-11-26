#include <ctime>
#include <string>
#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

std::string make_daytime_string()
{
    using namespace std;
    time_t now = time(0);
    return ctime(&now);
}

int main()
{
    try
    {
        boost::asio::io_context io_context;
        udp::socket socket(io_context,udp::endpoint(udp::v4,13));

        // 同步循环操作
        for(;;)
        {
            boost::array<char,1> recv_buf;
            udp::endpoint remote_endpoint;
            boost::system::error_code error;
            socket.receive_from(boost::asio::buffer(recv_buf),remote_endpoint);

            std::string message = make_daytime_string();
            boost::system::error_code ignored_error;
            socket.send_to(boost::asio::buffer(message),remote_endpoint,0,ignored_error);

        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}