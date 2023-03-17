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
  CUresult result = cuModuleLoadData(&module, kernels);
  std::cout << "module pointer: " << module << '\n';
  if (result != CUDA_SUCCESS || module == nullptr) {
    std::cerr << "Failed to load the embedded ptx with error: "
              << static_cast<unsigned int>(result) << '\n';
    return 1;
  }
}
