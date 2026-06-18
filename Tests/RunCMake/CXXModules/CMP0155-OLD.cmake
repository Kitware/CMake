enable_language(CXX)
unset(CMAKE_CXX_SCANDEP_SOURCE)

cmake_policy(SET CMP0155 OLD)

add_executable(cmp0155-old
  sources/module-use.cxx)
set_target_properties(cmp0155-old
  PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON)
