enable_language(CXX)
set(CMAKE_CXX_SCANDEP_SOURCE "echo")

cmake_policy(SET CMP0155 NEW)

add_executable(cmp0155-new-with-rule
  sources/module-use.cxx)
set_target_properties(cmp0155-new-with-rule
  PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON)
