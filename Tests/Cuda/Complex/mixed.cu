
#include <iostream>

#include "file1.h"
#include "file2.h"

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#define IMPORT __declspec(dllimport)
#else
#define EXPORT
#define IMPORT
#endif

result_type __device__ file1_func(int x);
result_type_dynamic __device__ file2_func(int x);

IMPORT void __host__ cuda_dynamic_lib_func();

static __global__ void mixed_kernel(result_type& r, int x)
{
  r = file1_func(x);
  result_type_dynamic rd = file2_func(x);
}

EXPORT int mixed_launch_kernel(int x)
{
  cuda_dynamic_lib_func();

  result_type r;
  mixed_kernel<<<1, 1>>>(r, x);
  return r.sum;
}
