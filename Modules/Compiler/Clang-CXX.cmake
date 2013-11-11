include(Compiler/Clang)
__compiler_clang(CXX)

if(NOT CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
  set(CMAKE_CXX_COMPILE_OPTIONS_VISIBILITY_INLINES_HIDDEN "-fvisibility-inlines-hidden")
endif()

cmake_policy(GET CMP0025 appleClangPolicy)
if(NOT appleClangPolicy STREQUAL NEW)
  return()
endif()
