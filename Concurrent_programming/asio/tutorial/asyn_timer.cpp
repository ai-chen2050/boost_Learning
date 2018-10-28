#include <iostream>
#include <boost/asio.hpp>

/* Using asio's asynchronous functionality means having a callback function 
 that will be called when an asynchronous operation completes. */
void print(const boost::system::error_code & /*e*/)
{
    std::cout<<"hello,world and Mr.chen !"<<std::endl;
}

int main()
{
    boost::asio::io_context io;
    boost::asio::steady_timer time1(io,boost::asio::chrono::seconds(3));

    // bind task to io 
    time1.async_wait(&print);
    io.run();

    return 0;
}