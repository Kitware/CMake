include(Compiler/ROCMClang)
__compiler_rocmclang(C)

set(_rocm_clang_ver "${CMAKE_C_COMPILER_VERSION_INTERNAL}")
set(CMAKE_C_COMPILER_VERSION "${CMAKE_C_COMPILER_VERSION_INTERNAL}")
include(Compiler/Clang-C)
set(CMAKE_C_COMPILER_VERSION "${_rocm_clang_ver}")
