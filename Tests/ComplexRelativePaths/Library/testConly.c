#include "testConly.h"

int CsharedFunction()
{
#if !defined(_WIN32) || defined(__CYGWIN__)
#ifndef TEST_C_FLAGS
  printf("TEST_C_FLAGS failed\n");
  return 0;
#else
  printf("Passed: TEST_C_FLAGS passed\n");  
#endif  
#endif
  return 1;
}
