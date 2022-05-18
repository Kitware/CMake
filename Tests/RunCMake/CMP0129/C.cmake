if(SET_CMP0129)
  cmake_policy(SET CMP0129 ${SET_CMP0129})
endif()

enable_language(C)
set(CMAKE_VERBOSE_MAKEFILE TRUE)
include(CompareCompilerVersion.cmake)
compare_compiler_version(C)
