#ifdef _MSC_VER
#  include "windows.h"
#else
#  define WINAPI
#endif

int WINAPI foo(void)
{
  return 10;
}

int bar(void)
{
  return 5;
}
