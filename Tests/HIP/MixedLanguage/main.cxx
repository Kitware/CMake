
#include <iostream>

#ifdef __HIP_PLATFORM_HCC__
#  error "__HIP_PLATFORM_HCC__ propagated to C++ compilation!"
#endif

#ifdef __HIP_ROCclr__
#  error "__HIP_ROCclr__ propagated to C++ compilation!"
#endif

#ifdef _WIN32
#  define IMPORT __declspec(dllimport)
#else
#  define IMPORT
#endif

extern "C" {
IMPORT int shared_c_func(int);
int static_c_func(int);
}

IMPORT int shared_cxx_func(int);
IMPORT int shared_hip_func(int);

int static_cxx_func(int);
int static_hip_func(int);

int main(int argc, char** argv)
{
  static_c_func(int(42));
  static_cxx_func(int(42));
  static_hip_func(int(42));

  shared_c_func(int(42));
  shared_cxx_func(int(42));
  shared_hip_func(int(42));

  return 0;
}
