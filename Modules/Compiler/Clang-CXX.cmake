include(Compiler/Clang)
__compiler_clang(CXX)

if(NOT CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
  set(CMAKE_CXX_COMPILE_OPTIONS_VISIBILITY_INLINES_HIDDEN "-fvisibility-inlines-hidden")
endif()

if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 2.1)
  set(CMAKE_CXX98_STANDARD_COMPILE_OPTION "-std=c++98")
  set(CMAKE_CXX98_EXTENSION_COMPILE_OPTION "-std=gnu++98")

  set(CMAKE_CXX11_STANDARD_COMPILE_OPTION "-std=c++0x")
  set(CMAKE_CXX11_EXTENSION_COMPILE_OPTION "-std=gnu++0x")
endif()

if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.1)
  set(CMAKE_CXX11_STANDARD_COMPILE_OPTION "-std=c++11")
  set(CMAKE_CXX11_EXTENSION_COMPILE_OPTION "-std=gnu++11")
endif()

set(CMAKE_CXX11_COMPILER_FEATURES)

include("${CMAKE_ROOT}/Modules/Compiler/CxxFeatureTesting.cmake")

macro(_get_clang_features std_version list)
  record_cxx_compiler_features("-std=${std_version}" ${list})
endmacro()

_get_clang_features(c++11 CMAKE_CXX11_COMPILER_FEATURES)

unset(testable_features)

set(CMAKE_CXX_COMPILER_FEATURES
  ${CMAKE_CXX11_COMPILER_FEATURES}
)
