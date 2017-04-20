
#include <iostream>

#include "file1.h"
#include "file2.h"

int file4_launch_kernel(int x);
int file5_launch_kernel(int x);

int main(int argc, char** argv)
{
  file4_launch_kernel(42);
  file5_launch_kernel(42);
  return 0;
}
