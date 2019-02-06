
#include <iostream>

static __global__ void file1_kernel(int in, int* out)
{
  *out = in * in;
}

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

int file1_launch_kernel()
{
  int ret = choose_cuda_device();
  if (ret) {
    return 0;
  }

  int input = 4;

  int* output;
  cudaError_t err = cudaMallocManaged(&output, sizeof(int));
  cudaDeviceSynchronize();
  if (err != cudaSuccess) {
    return 1;
  }

  file1_kernel<<<1, 1>>>(input, output);
  cudaDeviceSynchronize();
  err = cudaGetLastError();
  std::cout << err << " " << cudaGetErrorString(err) << std::endl;
  if (err == cudaSuccess) {
    // This kernel launch should failed as the device linking never occured
    std::cerr << "file1_kernel: kernel launch should have failed" << std::endl;
    return 1;
  }
  return 0;
}
