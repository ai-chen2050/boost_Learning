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
