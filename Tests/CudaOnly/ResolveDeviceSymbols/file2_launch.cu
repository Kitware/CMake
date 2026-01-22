
#include "file2.h"

static __global__ void file2_kernel(result_type_dynamic* r, int x)
{
  // call static_func which is a method that is defined in the
  // static library that is always out of date
  *r = file2_func(x);
}

int file2_launch_kernel(int x)
{
  result_type_dynamic* r;
  cudaMallocManaged(&r, sizeof(result_type_dynamic));

  file2_kernel<<<1, 1>>>(r, x);
  cudaDeviceSynchronize();

  auto sum = r->sum;
  cudaFree(r);

  return sum;
}
