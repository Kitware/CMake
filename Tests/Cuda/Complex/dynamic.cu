
#include <string>
#include <cuda.h>
#include <iostream>

int dynamic_base_func(int);

int __host__ cuda_dynamic_host_func(int x)
{
  return dynamic_base_func(x);
}

static
__global__
void DetermineIfValidCudaDevice()
{
}

void cuda_dynamic_lib_func()
{
  DetermineIfValidCudaDevice <<<1,1>>> ();
  cudaError_t err = cudaGetLastError();
  if(err == cudaSuccess)
    {
    std::cerr << cudaGetErrorString(err) << std::endl;
    }
}
