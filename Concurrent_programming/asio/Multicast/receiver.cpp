#include <iostream>
#include <array>
#include <string>
#include <boost/asio.hpp>

constexpr short multicast_port = 30001;

class Receiver
{
public:
    Receiver(boost::asio::io_context& io_context,
        const boost::asio::ip::address& listen_address,
        const boost::asio::ip::address& multicat_address)
    :_socket(io_context)
    {
        // Create the socket so that multiple may be bound to the same address.
        boost::asio::ip::udp::endpoint  listen_endpoint(listen_address,multicast_port);
        _socket.open(listen_endpoint.protocol());
        _socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
        _socket.bind(listen_endpoint);

        // Join the multicast group.
        _socket.set_option(boost::asio::ip::multicast::join_group(multicat_address));

        do_receive();
    }
private:
    void do_receive()
    {
        _socket.async_receive_from(
            boost::asio::buffer(_data),_send_endpoint,
            [this](boost::system::error_code ec,std::size_t length)
            {
                if(!ec)
                {
                    std::cout.write(_data.data(),length);
                    std::cout << std::endl;

                    do_receive();
                }
            }
        );
    }
private:
    boost::asio::ip::udp::socket _socket;
    boost::asio::ip::udp::endpoint _send_endpoint;
    std::array<char,1024> _data;
};  

int main(int argc,char* argv[])
{
    
    try
    {
        if(argc != 3)
        {
            std::cerr << "Usage: receiver <listen_address> <multicast_address>\n";
            std::cerr << "  For IPv4, try:\n";
            std::cerr << "    receiver 0.0.0.0 239.255.0.1\n";
            std::cerr << "  For IPv6, try:\n";
            std::cerr << "    receiver 0::0 ff31::8000:1234\n";
            return 1;
        }

        boost::asio::io_context io_context;
        Receiver r(io_context,
            boost::asio::ip::make_address(argv[1]),
            boost::asio::ip::make_address(argv[2]));
        
        io_context.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << '\n';
    }
    
}