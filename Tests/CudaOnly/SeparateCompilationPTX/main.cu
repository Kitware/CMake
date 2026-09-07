#include <iostream>

#include <cuda.h>

#include "embedded_objs.h"

#if defined(CUDA_VERSION) &&                                                  \
  CUDA_VERSION >= 13000 // get version from cuda.h header (clang cuda)
#  define CUDA_13_OR_GREATER
#elif defined(__CUDACC_VER_MAJOR__) &&                                        \
  __CUDACC_VER_MAJOR__ >= 13 // get version from nvcc compiler defines
#  define CUDA_13_OR_GREATER
#endif
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
#if defined(CUDA_13_OR_GREATER)
  CUctxCreateParams params = {};
  params.execAffinityParams = nullptr;
  params.numExecAffinityParams = 0;
  cuCtxCreate(&context, &params, 0, device);
#else
  cuCtxCreate(&context, 0, device);
#endif

  CUmodule module;
  CUresult result = cuModuleLoadData(&module, kernels);
  std::cout << "module pointer: " << module << '\n';
  if (result != CUDA_SUCCESS || module == nullptr) {
    std::cerr << "Failed to load the embedded ptx with error: "
              << static_cast<unsigned int>(result) << '\n';
    return 1;
  }
}
