#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif

int main()
{
#if defined(_WIN32)
  Sleep(10000);
#else
  sleep(10);
#endif
  return 0;
}
