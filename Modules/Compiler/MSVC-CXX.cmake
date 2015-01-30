# MSVC has no specific language level or flags to change it.
set(CMAKE_CXX_STANDARD_DEFAULT "")

macro(cmake_record_cxx_compile_features)
  record_compiler_features(CXX "" CMAKE_CXX_COMPILE_FEATURES)
endmacro()
