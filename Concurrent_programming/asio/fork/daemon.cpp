#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/signal_set.hpp>
#include <array>
#include <ctime>
#include <iostream>
#include <syslog.h>
#include <unistd.h>

using boost::asio::ip::udp;

class udp_daytime_server
{
public:
    udp_daytime_server(boost::asio::io_context& io_context)
    :_socket(io_context, {udp::v4(), 13})
    {
        receive();
    }

private:
    void receive()
    {
        _socket.async_receive_from(
            boost::asio::buffer(_recv_buffer),_remote_endpoint,
            [this](boost::system::error_code ec,std::size_t /*n*/)
            {
                if(!ec)
                {
                    using namespace std;    // For time_t, time and ctime 
                    time_t now = time(0);
                    std::string message = ctime(&now);

                    boost::system::error_code ignored_ec;
                    _socket.send_to(boost::asio::buffer(message),
                            _remote_endpoint,0,ignored_ec);
                }

                receive();
            });
    }

    udp::socket _socket;
    udp::endpoint _remote_endpoint;
    std::array<char, 1> _recv_buffer;
};

int main(int argc, char const *argv[])
{
    try
    {
        boost::asio::io_context io_context;

        udp_daytime_server server(io_context);

        // Register signal handlers so that the daemon may be shut down.
        boost::asio::signal_set signals(io_context,SIGINT,SIGTERM);
        signals.async_wait(
            [&](boost::system::error_code /*ec*/,int /*signo*/)
            {
                io_context.stop();
            }
        );

        // Inform the io_context that we are about to become a daemon.
        io_context.notify_fork(boost::asio::io_context::fork_perpare);

        // Fork the process and have the parent exit.Forking a new process is
        // also a prerequisite for the subsequent call to setsid().
        if(pid_t pid = fork())
        {
            if(pid > 0)
            {
                // We're in the parent process and need to exit.
                exit(0);
            }
            else  
            {
                syslog(LOG_ERR | LOG_USER, "First fork failed: %m");
                return 1;
            }
        }
        // Make the process a new session leader. This detaches it from the
        // terminal.
        setsid();

        chdir("/");

        umask(0);

        // A second fork ensures the process cannot acquire a controlling terminal.
        if(pid_t pid = fork())
        {
            if(pid > 0)
            {
                exit(0);
            }
            else 
            {
                syslog(LOG_ERR | LOG_USER, "Second fork failed: %m");
                return 1;
            }
        }
        // Close the standard streams. This decouples the daemon from the terminal
        // that started it.
        close(0);
        close(1);
        close(2);

        // We don't want the daemon to have any standard input.
        if (open("/dev/null", O_RDONLY) < 0)
        {
            syslog(LOG_ERR | LOG_USER, "Unable to open /dev/null: %m");
            return 1;
        }

        // Send standard output to a log file.
        const char* output = "/tmp/asio.daemon.out";
        const int flags = O_WRONLY | O_CREAT | O_APPEND;
        const mode_t mode = S_IRUSR | S_IWUSR | S_RGRP | S_IROTH;
        if (open(output, flags, mode) < 0)
        {
            syslog(LOG_ERR | LOG_USER, "Uable to open output file %s: %m",output);
            return 1;
        }

         // Also send standard error to the same log file.
         if (dup(1) < 0)
         {
            syslog(LOG_ERR | LOG_USER, "Unable to dup output descriptor: %m");
            return 1;
         }
        
        // Inform the io_context that we have finished becoming a daemon.
        syslog(LOG_INFO | LOG_USER,"Daemon started");
        io_context.run();
        syslog(LOG_INFO | LOG_USER, "Daemon stopped");
    }   
    catch (std::exception& e)
    {
        syslog(LOG_ERR | LOG_USER, "Exception: %s",e.what());
        std::cerr << "Exception: "<<　e.what() <<std::endl;
    } 
    
    return 0;
}
