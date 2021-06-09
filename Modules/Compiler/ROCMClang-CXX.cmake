include(Compiler/ROCMClang)
__compiler_rocmclang(CXX)

set(_rocm_clang_ver "${CMAKE_CXX_COMPILER_VERSION_INTERNAL}")
set(CMAKE_CXX_COMPILER_VERSION "${CMAKE_CXX_COMPILER_VERSION_INTERNAL}")
include(Compiler/Clang-CXX)
set(CMAKE_CXX_COMPILER_VERSION "${_rocm_clang_ver}")
