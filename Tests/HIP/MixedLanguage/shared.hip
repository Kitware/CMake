#include <stdexcept>
#include <type_traits>

#include <hip/hip_runtime_api.h>

#ifdef _WIN32
#  define EXPORT __declspec(dllexport)
#else
#  define EXPORT
#endif

static __global__ void fake_hip_kernel()
{
}

int __host__ shared_hip_func(int x)
{
  fake_hip_kernel<<<1, 1>>>();
  hipError_t err = hipGetLastError();
  if (err != hipSuccess) {
    throw std::runtime_error(hipGetErrorString(err));
  }
  return x * x + std::integral_constant<int, 17>::value;
}
