#include <system_error>
#include <type_traits>
#include <hip/hip_runtime_api.h>

static __global__ void fake_hip_kernel()
{
}


int __host__ interface_hip_func(int x)
{

  fake_hip_kernel<<<1, 1>>>();
  bool valid = (hipSuccess == hipGetLastError());
  if (!valid) {
    throw std::system_error(ENODEV, std::generic_category(), "no hip device");
  }
  return x * x + std::integral_constant<int, 17>::value;
}
