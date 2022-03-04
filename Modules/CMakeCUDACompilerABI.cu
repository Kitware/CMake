#ifndef __CUDACC__
#  error "A C or C++ compiler has been selected for CUDA"
#endif

#include <cstdio>

#include <cuda_runtime.h>

#include "CMakeCompilerABI.h"

int main(int argc, char* argv[])
{
  int require = 0;
  require += info_sizeof_dptr[argc];
  require += info_byte_order_big_endian[argc];
  require += info_byte_order_little_endian[argc];
#if defined(ABI_ID)
  require += info_abi[argc];
#endif
  static_cast<void>(argv);

  int count = 0;
  if (cudaGetDeviceCount(&count) != cudaSuccess || count == 0) {
    std::fprintf(stderr, "No CUDA devices found.\n");
    return -1;
  }

  int found = 0;
  const char* sep = "";
  for (int device = 0; device < count; ++device) {
    cudaDeviceProp prop;
    if (cudaGetDeviceProperties(&prop, device) == cudaSuccess) {
      std::printf("%s%d%d", sep, prop.major, prop.minor);
      sep = ";";
      found = 1;
    }
  }

  if (!found) {
    std::fprintf(stderr, "No CUDA architecture detected from any devices.\n");
    // Convince the compiler that the non-zero return value depends
    // on the info strings so they are not optimized out.
    return require ? -1 : 1;
  }

  return 0;
}
