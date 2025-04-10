#include <stdexcept>
#include <type_traits>

#include <hip/hip_runtime_api.h>

static __global__ void fake_hip_kernel()
{
}

int __host__ interface_hip_func(int x)
{
  fake_hip_kernel<<<1, 1>>>();
  hipError_t err = hipGetLastError();
  if (err != hipSuccess) {
    throw std::runtime_error(hipGetErrorString(err));
  }
  return x * x + std::integral_constant<int, 17>::value;
}
