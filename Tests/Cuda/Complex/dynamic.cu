
#include <string>
#include <cuda.h>

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

void cuda_dynamic_lib_func(std::string& contents )
{
  DetermineIfValidCudaDevice <<<1,1>>> ();
  if(cudaSuccess == cudaGetLastError())
    {
    contents = "ran a cuda kernel";
    }
  else
    {
    contents = "cant run a cuda kernel";
    }
}
