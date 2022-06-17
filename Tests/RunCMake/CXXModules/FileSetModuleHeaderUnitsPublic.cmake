enable_language(CXX)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
set(CMAKE_EXPERIMENTAL_CXX_SCANDEP_SOURCE "")

add_library(module-header
  sources/cxx-anchor.cxx)
target_sources(module-header
  PUBLIC
    FILE_SET fs TYPE CXX_MODULE_HEADER_UNITS FILES
      sources/module-header.h)
target_compile_features(module-header
  PRIVATE
    cxx_std_20)
