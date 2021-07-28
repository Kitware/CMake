
#include <type_traits>

#ifdef __HIP_PLATFORM_HCC__
#  error "__HIP_PLATFORM_HCC__ propagated to C++ compilation!"
#endif

#ifdef __HIP_ROCclr__
#  error "__HIP_ROCclr__ propagated to C++ compilation!"
#endif

#ifdef _WIN32
#  define EXPORT __declspec(dllexport)
#else
#  define EXPORT
#endif

EXPORT int shared_cxx_func(int x)
{
  return x * x + std::integral_constant<int, 14>::value;
}
