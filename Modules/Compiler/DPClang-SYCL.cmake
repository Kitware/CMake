include(Compiler/DPClang)
__compiler_dpclang(SYCL)

include(Compiler/IntelLLVM-SYCL-DeviceLink)

if(CMAKE_SYCL_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
  set(CMAKE_SYCL_COMPILE_OPTIONS_SEPARABLE_COMPILATION_ON "-clang:-fgpu-rdc")
  set(CMAKE_SYCL_COMPILE_OPTIONS_SEPARABLE_COMPILATION_OFF "-clang:-fno-gpu-rdc")
  set(CMAKE_SYCL_DEVICE_LINK_OPTIONS "-fsycl;-clang:-fsycl-link")
  set(CMAKE_SYCL_DEVICE_LINK_OPTIONS_WHOLE_ARCHIVE "-clang:-Wl,--whole-archive")
  set(CMAKE_SYCL_DEVICE_LINK_OPTIONS_NO_WHOLE_ARCHIVE "-clang:-Wl,--no-whole-archive")
endif()
