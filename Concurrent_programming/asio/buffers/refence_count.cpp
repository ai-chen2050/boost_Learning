#include <ctime>
#include <string>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

// A reference-counted non-modifiable buffer class.
class shared_const_buffer
{
public:
    // Construct from a std::string.
    explicit shared_const_buffer(const std::string& data)
    :_data(new std::vector<char>(data.begin(),data.end()))
    ,_buffer(boost::asio::buffer(*_data))
    {

    }

    // Implement the ConstBufferSequence requirements.
    typedef boost::asio::const_buffer value_type;
    typedef const boost::asio::const_buffer* const_iterator;

    const boost::asio::const_buffer* begin()const { return &_buffer; }
    const boost::asio::const_buffer* end()const {   return &_buffer + 1; }

private:
    std::shared_ptr<std::vector<char> > _data;
    boost::asio::const_buffer _buffer;
};

class session : public std::enable_shared_from_this<session>
{
public:
    session(tcp::socket socket)
    :_socket(std::move(socket))
    {

    }

    void start()
    {
        do_write();
    }
private:
    void do_write()
    {
        std::time_t now = std::time(0);
        shared_const_buffer buffer(std::ctime(&now));

        auto self(shared_from_this());
        boost::asio::async_write(_socket,buffer,
        [this,self](boost::system::error_code /* ec */,std::size_t  /*length*/)
        {

        });
    }

    // The socket used to communicate with the client.
    tcp::socket _socket;
}

class server 
{
public:
    server(boost::asio::io_context& io_context,short port)
    :_acceptor(io_context,tcp::endpoint(tcp::v4(),port))
    {
        do_accept();
    }
private:
    void do_accept()
    {
        _acceptor.async_accept(
            [this](boost::system::error_code ec,tcp::socket socket)
            {
                if(!ec)
                {
                    std::make_shared<session>(std::move(socket))->start();
                }

                do_accept();
            }
        );  
    }

    tcp::acceptor _acceptor;
};

int main(int argc,char* argv[])
{
    
    try
    {
        if(argc != 2)
        {
            std::cerr << "Usage: reference_counted <port>\n";
            return 1;
        }

        boost::asio::io_context io_context;
        server s(io_context,std::atoi(argv[1]));

        io_context.run();   
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}