#include <windows.h>

class Windows_handle
{
public:
   Windows_handle(HANDLE h)
   :_handle(h)
   {
      
   }
   ~Windows_handle()
   {
      CloseHandle(_handle);
   }
   HANDLE handle()const 
   {
      return _handle;
   }
private:
   HANDLE _handle;
}; 

int main(int argc, char const *argv[])
{
   Windows_handle h(OpenProcess(PROCESS_SET_INFORMATION,FALSE,GetCurrentProcessId()));
   SetPriorityClass(h.handle(),HIGH_PRIORITY_CLASS);
   return 0;
}
