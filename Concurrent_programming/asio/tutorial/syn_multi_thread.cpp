/* Introduction */

// #  -*- coding:utf-8 -*- 
// #  @Author: Mr.chen(ai-chen2050@qq.com) 
// #  @Date: 2018-11-08 23:28:16 
  
/* Consequently, calling io_context::run() from only one thread ensures that callback 
handlers cannot run concurrently.The single threaded approach is usually the best 
place to start when developing applications using asio. The downside is the limitations 
it places on programs, particularly servers, including:

1、Poor responsiveness when handlers can take a long time to complete.
2、An inability to scale on multiprocessor systems.

So, an alternative approach is to have a pool of threads calling io_context::run(). 
However, as this allows handlers to execute concurrently, we need a method of synchronisation 
when handlers might be accessing a shared, thread-unsafe resource.  ==> io_context::strand. */

#include <iostream>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>


// Define a class will extend the previous tutorial by running two timers in parallel.
class printer
{
public:
     printer(boost::asio::io_context& io)
     :_strand(io)
     ,_timer1(io,boost::asio::chrono::seconds(1))
     ,_timer2(io,boost::asio::chrono::seconds(1))
     ,_count(0)
     {
         _timer1.async_wait(boost::asio::bind_executor(_strand,boost::bind(&printer::print1,this)));
         _timer2.async_wait(boost::asio::bind_executor(_strand,boost::bind(&printer::print2,this)));
     }

    ~printer()
    {
        std::cout<< "Final count is "<<_count <<std::endl;
    }

    // In a multithreaded program, the handlers for asynchronous operations should be synchronised if they access shared resources. 
    // In this tutorial, the shared resources used by the handlers (print1 and print2) are std::cout and the count_ data member.
    void print1()
    {
        if(_count < 10)
        {
            std::cout<< "The timer1: "<<_count <<std::endl;
            ++ _count;
            _timer1.expires_at(_timer1.expiry() + boost::asio::chrono::seconds(1));
            _timer1.async_wait(boost::asio::bind_executor(_strand,boost::bind(&printer::print1,this)));
        }
    }

    void print2()
    {
        if (_count < 10)
        {
            std::cout << "The timer2: "<<_count << std::endl;
            ++ _count;
            _timer2.expires_at(_timer2.expiry() + boost::asio::chrono::seconds(1));
            _timer2.async_wait(boost::asio::bind_executor(_strand,boost::bind(&printer::print2,this)));
        }
    }
private:
    boost::asio::io_context::strand _strand;
    boost::asio::steady_timer _timer1;
    boost::asio::steady_timer _timer2;
    int _count;
};

int main()
{
    boost::asio::io_context io;
    printer p(io);
    // create thread 
    boost::thread t(boost::bind(&boost::asio::io_context::run,&io));
    // main thread io_context::run()
    io.run();
    t.join();

    return 0;
}