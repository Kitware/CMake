#include <iostream>

#include <cuda.h>

#include "embedded_objs.h"

int main()
{
  cuInit(0);
  int count = 0;
  cuDeviceGetCount(&count);
  if (count == 0) {
    std::cerr << "No CUDA devices found\n";
    return 1;
  }

  CUdevice device;
  cuDeviceGet(&device, 0);

  CUcontext context;
  cuCtxCreate(&context, 0, device);

  CUmodule module;
  cuModuleLoadData(&module, kernels);
  if (module == nullptr) {
    std::cerr << "Failed to load the embedded ptx" << std::endl;
    return 1;
  }
  std::cout << module << std::endl;
}
