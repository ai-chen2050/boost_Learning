#include <iostream>
#include <boost/asio.hpp>

int main()
{
    boost::asio::io_context io;
    boost::asio::steady_timer timer1(io,boost::asio::chrono::seconds(5));

    timer1.wait();

    std::cout<< "hello world and Mr.chen !"<< std::endl;
    
    return 0;
}

// 编译 Shell(之后同) :  g++ syn_timer.cpp -o syn_timer  -lboost_system  -lpthread