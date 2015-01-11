include(Platform/Windows-MSVC)
set(_COMPILE_CXX " /TP")
if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 18.0)
  set(_FS_CXX " /FS")
endif()
__windows_compiler_msvc(CXX)

# No version of MSVC has full conformance to C++11. Therefore the
# __cplusplus macro always evaluates to 98 even if the compilers come with
# C++11/14/+ features enabled.
set(CMAKE_CXX_STANDARD_DEFAULT 98)

macro(cmake_record_cxx_compile_features)
  record_compiler_features(CXX "" CMAKE_CXX_COMPILE_FEATURES)
endmacro()
