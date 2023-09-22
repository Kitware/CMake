
#include <iostream>

#ifdef _WIN32
#  define EXPORT __declspec(dllexport)
#else
#  define EXPORT __attribute__((__visibility__("default")))
#endif

__global__ void k1()
{
}

EXPORT int foo()
{
  k1<<<1, 1>>>();
  return 0;
}
