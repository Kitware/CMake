
#include <stdexcept>
#include <cmath>
#include <math.h>
#include <memory>

#include <hip/hip_runtime.h>
#include <hip/hip_fp16.h>

namespace {
template<class T, class F>
__global__ void global_entry_point(F f, T *out) {
  *out = f();
}

template <class T, class F>
bool verify(F f, T expected)
{
  std::unique_ptr<T> cpu_T(new T);
  T* gpu_T = nullptr;
  hipMalloc((void**)&gpu_T, sizeof(T));
  hipLaunchKernelGGL(global_entry_point, 1, 1, 0, 0, f, gpu_T);
  hipMemcpy(cpu_T.get(), gpu_T, sizeof(T), hipMemcpyDeviceToHost);
  hipFree(gpu_T);
  return (*cpu_T == expected);
}
}

int main(int argc, char** argv)
{
  bool valid = verify([]__device__(){ return std::round(1.4f); }, 1.0f);
  valid &= verify([]__device__(){ return max<_Float16>(1.0f, 2.0f); }, 2.0f);
  valid &= verify([]__device__(){ return min<_Float16>(1.0f, 2.0f); }, 1.0f);

  if (valid) {
    return 0;
  } else {
    return 1;
  }
}
