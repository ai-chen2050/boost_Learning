// 不可共享的指针
#include <iostream>
#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>

int main(int argc, char const *argv[])
{
   boost::scoped_ptr<int> i(new int);
   *i = 1;
   *i.get() = 2;
   i.reset(new int);
   std::cout << *i << std::endl;

   boost::scoped_array<int> array_scope(new int[2]);
   *array_scope.get() = 1;
   array_scope[1] = 2;
   array_scope.reset(new int[4]);

   std::cout << "array_scope size is " << sizeof(array_scope) << std::endl;

   return 0;
}
