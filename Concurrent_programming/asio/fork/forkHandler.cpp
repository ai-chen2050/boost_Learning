#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using boost::asio::ip::tcp;

class server
{
public: 
    server(boost::asio::io_context& io_context,unsigned short port)
    :_io_context(io_context)
    ,_signal(io_context,SIGCHLD)
    ,_acceptor(io_context,{tcp::v4(),port})
    ,_socket(io_context)
    {
        wait_for_signal();
        accept();
    }
private:
    void wait_for_signal()
    {
        _signal.async_wait(
            [this](boost::system::error_code /*ec*/,int /*signo*/)
            {
                // Only the parent process should check for this signal. We can
                // determine whether we are in the parent by checking if the acceptor
                // is still open.
                if(_acceptor.is_open())
                {
                    // Reap completed child processes so that we don't end up with
                    // zombies.
                    int status = 0;
                    while(waitpid(-1,&status,WNOHANG) > 0) {}

                    wait_for_signal();
                }
            }
        );
    }

    void accept()
    {
        _acceptor.async_accept(
            [this](boost::system::error_code ec,tcp::socket new_socket)
            {
                if(!ec)
                {
                    // Take ownership of the newly accepted socket.
                    _socket = std::move(new_socket);

                    // Inform the io_context that we are about to fork. The io_context
                    // cleans up any internal resources, such as threads, that may
                    // interfere with forking.
                    _io_context.notify_fork(boost::asio::io_context::fork_prepare);

                    if(fork() == 0)
                    {
                        // Inform the io_context that the fork is finished and that this
                        // is the child process. The io_context uses this opportunity to
                        // create any internal file descriptors that must be private to
                        // the new process.
                        _io_context.notify_fork(boost::asio::io_context::fork_child);

                        // The child won't be accepting new connections, so we can close
                        // the acceptor. It remains open in the parent.
                        _acceptor.close();

                        // The child process is not interested in processing the SIGCHLD
                        // signal.
                        _signal.cancel();

                        read();
                    }
                    else
                    {
                        // Inform the io_context that the fork is finished (or failed)
                        // and that this is the parent process. The io_context uses this
                        // opportunity to recreate any internal resources that were
                        // cleaned up during preparation for the fork.
                        _io_context.notify_fork(boost::asio::io_context::fork_parent);

                        // The parent process can now close the newly accepted socket. It
                        // remains open in the child.
                        _socket.close();

                        accept();
                    }
                }
                else
                {
                    std::cout << "Accept error: "<< ec.message() << std::endl;  
                    accept();
                }
            }
        );
    }

    void read()
    {
        _socket.async_read_some(boost::asio::buffer(_data),
        [this](boost::system::error_code ec,std::size_t length)
        {
            if(!ec)
                write(length);
        });
    }

    void write(std::size_t length)
    {
        boost::asio::async_write(_socket,boost::asio::buffer(_data,length),
            [this](boost::system::error_code ec,std::size_t /*length*/)
            {
                if(!ec)
                    read();
            });
    }

    boost::asio::io_context& _io_context;
    boost::asio::signal_set _signal;
    tcp::acceptor _acceptor;
    tcp::socket _socket;
    std::array<char,1024> _data;    // 缓存数组
};

int main(int argc, char const *argv[])
{
    try
    {
        if(argc != 2)
        {
            std::cout << "Usage: process_per_connection \n";
            return 1;
        }

        boost::asio::io_context io_context;     // 资源尽量在外部定义, 封装的类里面存放指针或者引用即可

        using namespace std;    // For atoi
        server s(io_context,atoi(argv[1]));

        io_context.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}


