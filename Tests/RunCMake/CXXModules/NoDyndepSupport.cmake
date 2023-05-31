enable_language(CXX)
set(CMAKE_EXPERIMENTAL_CXX_SCANDEP_SOURCE "")

if (NOT CMAKE_CXX_STANDARD_DEFAULT)
  set(CMAKE_CXX_STANDARD_DEFAULT "11")
endif ()

add_library(nodyndep)
target_sources(nodyndep
  PUBLIC
    FILE_SET fs TYPE CXX_MODULES FILES
      sources/module.cxx)
target_compile_features(nodyndep
  PRIVATE
    cxx_std_20)
