#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

class printer 
{
public: 
    printer(boost::asio::io_context& io)
    :_timer(io,boost::asio::chrono::seconds(1))
    ,_count(0)
    {
        _timer.async_wait(boost::bind(&printer::print,this));
    }

    ~printer()
    {
        std::cout<< "Final count is "<<_count <<std::endl;
    }

    void print()
    {
        if(_count < 5)
        {
            std::cout << "The count is "<< _count << std::endl;
            ++ _count;

            _timer.expires_at(_timer.expiry() + boost::asio::chrono::seconds(1));
            _timer.async_wait(boost::bind(&printer::print, this));
        }
    }
private:
    boost::asio::steady_timer _timer;
    int _count;
};

int main()
{
    boost::asio::io_context io;
    printer p(io);
    
    io.run();

    return 0;
}