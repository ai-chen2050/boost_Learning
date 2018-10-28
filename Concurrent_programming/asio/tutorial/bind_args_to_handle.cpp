#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

// To implement a repeating timer using asio,
// we add two new parameters to the print function:
void print(const boost::system::error_code& /*e*/,
            boost::asio::steady_timer* t,int* count)
{
    if(*count < 5)
    {
        std::cout<< "The count is "<< *count <<std::endl;
        ++(*count);

        // Next we move the expiry time for the timer along, use the same timer 
        t->expires_at(t->expiry() + boost::asio::chrono::seconds(1));
        t->async_wait(boost::bind(print,
                boost::asio::placeholders::error, t, count));
    }
}

int main()
{
    boost::asio::io_context io;

    // count for exit 
    int count = 0;
    boost::asio::steady_timer t(io,boost::asio::chrono::seconds(1));
    t.async_wait(boost::bind(print,
            boost::asio::placeholders::error,&t,&count));
    
    // start io 
    io.run();

    // validate count 
    std::cout<< "Final count is " <<count << std::endl;

    return 0;
}