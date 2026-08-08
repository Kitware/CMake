#include <cstdio>

#include "src.h"

__global__ void hello_world_kernel()
{
  printf("Hello from GPU thread %d\n", threadIdx.x);
}

void hello_world()
{
  hello_world_kernel<<<1, 1>>>();
  cudaDeviceSynchronize();
}
