
#include <iostream>

#include "file1.h"
#include "file2.h"

result_type __device__ file1_func(int x);
result_type_dynamic __device__ file2_func(int x);

static __global__ void file3_kernel(result_type& r, int x)
{
  r = file1_func(x);
  result_type_dynamic rd = file2_func(x);
}

int file3_launch_kernel(int x)
{
  result_type r;
  file3_kernel<<<1, 1>>>(r, x);
  cudaError_t err = cudaGetLastError();
  if (err == cudaSuccess) {
    std::cerr << cudaGetErrorString(err) << std::endl;
    return x;
  }
  return r.sum;
}
