#include <iostream>

#include "file1.h"
#include "file2.h"

#ifdef _WIN32
#define IMPORT __declspec(dllimport)
#else
#define IMPORT
#endif

IMPORT int call_cuda_seperable_code(int x);
IMPORT int mixed_launch_kernel(int x);

int main(int argc, char** argv)
{
  call_cuda_seperable_code(42);
  mixed_launch_kernel(42);
  return 0;
}
