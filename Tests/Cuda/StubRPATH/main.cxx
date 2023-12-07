
#include <iostream>

#include <cuda.h>

int main(int argc, char** argv)
{
  int nDevices = 0;
  cuInit(0);
  auto err = cuDeviceGetCount(&nDevices);
  if (err != CUDA_SUCCESS) {
    std::cerr << "Failed to retrieve the number of CUDA enabled devices "
              << err << std::endl;
    return 1;
  }
  return 0;
}
