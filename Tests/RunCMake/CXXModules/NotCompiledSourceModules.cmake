enable_language(CXX)
set(CMAKE_CXX_SCANDEP_SOURCE "")

add_library(not-cxx-source)
target_sources(not-cxx-source
  PRIVATE
    sources/cxx-anchor.cxx
  PUBLIC
    FILE_SET fs TYPE CXX_MODULES FILES
      sources/not-compiled.txt)
target_compile_features(not-cxx-source
  PRIVATE
    cxx_std_20)
