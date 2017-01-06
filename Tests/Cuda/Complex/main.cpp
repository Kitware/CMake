#include <iostream>

#include "file1.h"
#include "file2.h"

int call_cuda_seperable_code(int x);
int mixed_launch_kernel(int x);

int main(int argc, char** argv)
{
  call_cuda_seperable_code(42);
  mixed_launch_kernel(42);
  return 0;
}
