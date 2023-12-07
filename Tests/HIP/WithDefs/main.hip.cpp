#include <iostream>

#include <hip/hip_runtime_api.h>
#include <inc_hip.h>
#ifndef INC_HIP
#  error "INC_HIP not defined!"
#endif

#ifndef PACKED_DEFINE
#  error "PACKED_DEFINE not defined!"
#endif

#ifndef FLAG_COMPILE_LANG_HIP
#  error "FLAG_COMPILE_LANG_HIP not defined!"
#endif

#ifndef FLAG_LANG_IS_HIP
#  error "FLAG_LANG_IS_HIP not defined!"
#endif

#if !FLAG_LANG_IS_HIP
#  error "Expected FLAG_LANG_IS_HIP"
#endif

#ifndef DEF_COMPILE_LANG_HIP
#  error "DEF_COMPILE_LANG_HIP not defined!"
#endif

#ifndef DEF_LANG_IS_HIP
#  error "DEF_LANG_IS_HIP not defined!"
#endif

#if !DEF_LANG_IS_HIP
#  error "Expected DEF_LANG_IS_HIP"
#endif

#ifndef DEF_HIP_COMPILER
#  error "DEF_HIP_COMPILER not defined!"
#endif

#ifndef DEF_HIP_COMPILER_VERSION
#  error "DEF_HIP_COMPILER_VERSION not defined!"
#endif

static __global__ void DetermineIfValidHIPDevice()
{
}

#ifdef _MSC_VER
#  pragma pack(push, 1)
#  undef PACKED_DEFINE
#  define PACKED_DEFINE
#endif
#ifdef __NVCC__
#  undef PACKED_DEFINE
#  define PACKED_DEFINE
#endif
struct PACKED_DEFINE result_type
{
  bool valid;
  int value;
#if defined(NDEBUG) && !defined(DEFREL)
#  error missing DEFREL flag
#endif
};
#ifdef _MSC_VER
#  pragma pack(pop)
#endif

result_type can_launch_kernel()
{
  result_type r;
  DetermineIfValidHIPDevice<<<1, 1>>>();
  r.valid = (hipSuccess == hipGetLastError());
  if (r.valid) {
    r.value = 1;
  } else {
    r.value = -1;
  }
  return r;
}

int main(int argc, char** argv)
{
  hipError_t err;
  int nDevices = 0;
  err = hipGetDeviceCount(&nDevices);
  if (err != hipSuccess) {
    std::cerr << hipGetErrorString(err) << std::endl;
    return 1;
  }
  return 0;
}
