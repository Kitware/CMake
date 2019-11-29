
#include <iostream>

#include "cuda.h"

#ifdef _WIN32
#  define IMPORT __declspec(dllimport)
#else
#  define IMPORT
#endif

#ifndef _WIN32
IMPORT int file1_launch_kernel(int x);
#endif

IMPORT int file2_launch_kernel(int x);

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
    if (prop.major >= 3) {
      err = cudaSetDevice(i);
      if (err != cudaSuccess) {
        std::cout << "Could not select CUDA device " << i << std::endl;
      } else {
        return 0;
      }
    }
  }

  std::cout << "Could not find a CUDA enabled card supporting compute >=3.0"
            << std::endl;

  return 1;
}

int main(int argc, char** argv)
{
  int ret = choose_cuda_device();
  if (ret) {
    return 0;
  }

  cudaError_t err;
#ifndef _WIN32
  file1_launch_kernel(1);
  err = cudaGetLastError();
  if (err != cudaSuccess) {
    std::cerr << "file1_launch_kernel: kernel launch should have passed.\n "
                 "Error message: "
              << cudaGetErrorString(err) << std::endl;
    return 1;
  }
#endif

  file2_launch_kernel(1);
  err = cudaGetLastError();
  if (err != cudaSuccess) {
    std::cerr << "file2_launch_kernel: kernel launch should have passed.\n "
                 "Error message: "
              << cudaGetErrorString(err) << std::endl;
    return 1;
  }

  return 0;
}
