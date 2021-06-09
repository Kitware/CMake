include(Compiler/ROCMClang)
__compiler_rocmclang(OBJC)

set(_rocm_clang_ver "${CMAKE_OBJC_COMPILER_VERSION_INTERNAL}")
set(CMAKE_OBJC_COMPILER_VERSION "${CMAKE_OBJC_COMPILER_VERSION_INTERNAL}")
include(Compiler/Clang-OBJC)
set(CMAKE_OBJC_COMPILER_VERSION "${_rocm_clang_ver}")
