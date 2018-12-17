#ifndef HTTP_HEADER_HPP 
#define HTTP_HEADER_HPP

#include <string>

namespace http  
{
namespace server        
{

struct header   
{
    std::string name;
    std::string value;
};

}// end of namespace server
} // end of namspace http

#endif // !HTTP_HEADER_HPP  