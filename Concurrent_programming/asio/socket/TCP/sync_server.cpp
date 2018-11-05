#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

// define a function to create the string to be sent back to the client. 
std::string make_daytime_string()
{
    using namespace std;        // for time_t, time, and ctime;
    time_t now = time(0);
    return ctime(&now);
}

int main()
{
    try
    {
        boost::asio::io_context io_context;
        // define a acceptor to listen for new connections
        tcp::acceptor acceptor(io_context,tcp::endpoint(tcp::v4(),13));

        // define a iterative server, it will handle one connection at a time.
        for(;;)
        {
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            std::string message = make_daytime_string();

            boost::system::error_code ignored_error;
            boost::asio::write(socket,boost::asio::buffer(message),ignored_error);
        }
    }
    catch (std::exception& e) 
    {
        std::cerr << e.what() <<std::endl;
    }

    return 0;
}