enable_language(CXX)
unset(CMAKE_CXX_SCANDEP_SOURCE)

cmake_policy(SET CMP0155 NEW)

add_executable(cmp0155-new
  sources/module-use.cxx)
set_target_properties(cmp0155-new
  PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON)
