
#include <iostream>

#ifdef __HIP_PLATFORM_HCC__
#  error "__HIP_PLATFORM_HCC__ propagated to C++ compilation!"
#endif

#ifdef __HIP_ROCclr__
#  error "__HIP_ROCclr__ propagated to C++ compilation!"
#endif

int interface_hip_func(int);

int main(int argc, char** argv)
{
  interface_hip_func(int(42));

  return 0;
}
