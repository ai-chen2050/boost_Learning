#include <array>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

// Class to manage the memory to be used for handler-based custom allocation.
class handler_memory 
{
public:
    handler_memory()    
    :_is_use(false)
    {

    }

    // delete copy construct and operator= 
    handler_memory(const handler_memory&)=delete;
    handler_memory& operator=(const handler_memory&)=delete;

    void* allocate(std::size_t size)
    {
        if(!_is_use && size < sizeof(_storage))
        {
            _is_use = true;
            return &_storage;
        }
        else 
        {
            return ::operator new(size);    // allocator delegates allocation to the global heap.
        }
    }    

    void deallocate(void* pointer)
    {
        if(pointer == &_storage)
        {
            _is_use = false;
        }
        else 
        {
            ::operator delete(pointer);
        }
    }
private:
    // Storage space used for handler-based custom memory allocation.
    typename std::aligned_storage<1024>::type _storage;

    // whether has been used    
    bool _is_use;   
};

// The allocator to be associated with the handler objects.
template <typename T>
class handler_allocator 
{
public:
    using value_type = T;

    explicit handler_allocator(handler_memory& mem)
    :_memory(mem)
    {

    }

    template <typename U>
    handler_allocator(const handler_allocator<U>& other)noexcept
    :_memory(other._memory)
    {

    }    

    bool operator==(const handler_allocator& other)const noexcept
    {
        return &_memory == &other._memory;
    }

    bool operator!=(const handler_allocator& other)const noexcept
    {
        return &_memory != &other._memory;
    }

    T* allocate(std::size_t n)const
    {
        return static_cast<T*>(_memory.allocate(sizeof(T) * n));
    }

    void deallocate(T* p,std::size_t /*n*/)const
    {
        return _memory.deallocate(p);
    }

private:
    // 让实例化模板类称为模板的友元
    template <typename> friend class handler_allocator;

    // The underlying memory.
    handler_memory& _memory;
};

// Wrapper class template for handler objects to allow handler memory allocation to be customised.
template <typename Handler>
class custom_alloc_handler 
{
public:
    using allocator_type = handler_allocator<Handler>;

    custom_alloc_handler(handler_memory& m,Handler h)
    :_memory(m)
    ,_handler(h)
    {

    }

    allocator_type get_allocator()const noexcept
    {
        return allocator_type(_memory);
    }

    template <typename ...Args>
    void operator()(Args&&... args)
    {
        _handler(std::forward<Args>(args)...);
    }

private:
    handler_memory& _memory;
    Handler _handler;
};


// Helper function to wrap a handler object to add custom allocation.
template <typename Handler>
inline custom_alloc_handler<Handler> make_custom_alloc_handler(handler_memory& m,Handler h)
{
    return custom_alloc_handler<Handler>(m,h);
}

class session : public std::enable_shared_from_this<session>
{
public:
    session(tcp::socket socket)
    :_socket(std::move(socket))
    {

    }

    void start()
    {
        do_read();
    }
private:
    void do_read()
    {
        auto self(shared_from_this());
        _socket.async_read_some(boost::asio::buffer(_data),
            make_custom_alloc_handler(_handler_memory,
                [this,self](boost::system::error_code ec,std::size_t length)
                {
                    if(!ec)
                    {
                        do_write(length);
                    }
                }));
    }

    void do_write(std::size_t length)
    {
        auto self(shared_from_this());
        boost::asio::async_write(_socket,boost::asio::buffer(_data,length),
        make_custom_alloc_handler(_handler_memory,
        [this,self](boost::system::error_code ec,std::size_t /*lengyh*/)
        {
            if(!ec)
            {
                do_read();
            }
        }));
    }

    // The socket used to communicate with the client.
    tcp::socket _socket;

    // Buffer used to store data received from the client.
    std::array<char, 1024> _data;

    // The memory to use for handler-based custom memory allocation.
    handler_memory  _handler_memory;
};

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
                    // 相当于构造一个匿名函数   
                    std::make_shared<session>(std::move(socket))->start();
                }
                do_accept();
            }
        );
    }

    tcp::acceptor _acceptor;
};

int main(int argc, char const *argv[])
{
    
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage:server <port>\n";
            return 1;
        }

        boost::asio::io_context io_context;
        server(io_context,std::atoi(argv[1]));
        io_context.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
        
    return 0;
}
