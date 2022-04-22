#include <iostream>

#include "cuda.h"

#ifdef _WIN32
#  define IMPORT __declspec(dllimport)
#else
#  define IMPORT
#endif

IMPORT int launch_kernel(int x);

int choose_cuda_device()
{
  int nDevices = 0;
  cudaError_t err = cudaGetDeviceCount(&nDevices);
  if (err != cudaSuccess) {
    std::cerr << "Failed to retrieve the number of CUDA enabled devices"
              << std::endl;
    return 1;
  }
  for (int i = 0; i < nDevices; ++i) {
    cudaDeviceProp prop;
    cudaError_t err = cudaGetDeviceProperties(&prop, i);
    if (err != cudaSuccess) {
      std::cerr << "Could not retrieve properties from CUDA device " << i
                << std::endl;
      return 1;
    }
    std::cout << "prop.major: " << prop.major << std::endl;
    err = cudaSetDevice(i);
    if (err != cudaSuccess) {
      std::cout << "Could not select CUDA device " << i << std::endl;
    } else {
      return 0;
    }
  }

  std::cout << "Could not find a CUDA enabled card" << std::endl;

  return 1;
}

int main()
{
  int ret = choose_cuda_device();
  if (ret) {
    return 0;
  }

  cudaError_t err;
  launch_kernel(1);
  err = cudaGetLastError();
  if (err != cudaSuccess) {
    std::cerr << "launch_kernel: kernel launch should have passed.\n "
                 "Error message: "
              << cudaGetErrorString(err) << std::endl;
    return 1;
  }

  return 0;
}
