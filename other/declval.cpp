#include <utility>
#include <iostream>

struct Default 
{
   int foo() const 
   {
      return 1;
   }
};

struct NoDefault 
{
   NoDefault(const NoDefault& );
   int foo() const
   {
      return 1;
   } 
};

int main(int argc, char const *argv[])
{
   decltype(Default().foo()) n1 = 1;      // type of n1 is int

   //  decltype(NonDefault().foo()) n2 = n1;               // error: no default constructor
   decltype(std::declval<NoDefault>().foo()) n2 = n1;      // type of n2 is int
   
   std::cout << "\nn1 = " << n1 << '\n'
             << "n2 = " << n2 << '\n';
   return 0;
}
