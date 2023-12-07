enable_language(CXX)
unset(CMAKE_CXX_SCANDEP_SOURCE)

add_executable(noscanning-target-property
  sources/module-use.cxx)
set_target_properties(noscanning-target-property
  PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_SCAN_FOR_MODULES 1)
