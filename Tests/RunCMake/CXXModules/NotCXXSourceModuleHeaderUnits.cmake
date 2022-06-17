enable_language(C)
enable_language(CXX)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
set(CMAKE_EXPERIMENTAL_CXX_SCANDEP_SOURCE "")

add_library(not-cxx-source)
target_sources(not-cxx-source
  PRIVATE
    sources/cxx-anchor.cxx
  PUBLIC
    FILE_SET fs TYPE CXX_MODULE_HEADER_UNITS FILES
      sources/c-anchor.c)
target_compile_features(not-cxx-source
  PRIVATE
    cxx_std_20)
