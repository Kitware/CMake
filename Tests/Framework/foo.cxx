#include <stdio.h>
#ifdef _WIN32
#  define CM_TEST_LIB_EXPORT  __declspec( dllexport )
#endif
CM_TEST_LIB_EXPORT void foo()
{
  printf("foo\n");
}
