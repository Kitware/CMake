enable_language(C)
enable_language(CXX)
set(CMAKE_CXX_SCANDEP_SOURCE "")

add_library(not-cxx-source)
target_sources(not-cxx-source
  PUBLIC
    FILE_SET fs TYPE CXX_MODULES FILES
      sources/c-anchor.c)
target_compile_features(not-cxx-source
  PRIVATE
    cxx_std_20)
