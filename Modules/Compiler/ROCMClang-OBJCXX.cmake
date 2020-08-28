include(Compiler/ROCMClang)
__compiler_rocmclang(OBJCXX)

set(_rocm_clang_ver "${CMAKE_OBJCXX_COMPILER_VERSION_INTERNAL}")
set(CMAKE_OBJCXX_COMPILER_VERSION "${CMAKE_OBJCXX_COMPILER_VERSION_INTERNAL}")
include(Compiler/Clang-OBJCXX)
set(CMAKE_OBJCXX_COMPILER_VERSION "${_rocm_clang_ver}")
