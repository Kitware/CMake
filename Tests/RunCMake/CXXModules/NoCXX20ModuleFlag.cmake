enable_language(CXX)

set(CMAKE_EXPERIMENTAL_CXX_SCANDEP_SOURCE "echo")

add_library(noexperimentalflag)
target_sources(noexperimentalflag
  PUBLIC
    FILE_SET fs TYPE CXX_MODULES FILES
      sources/module.cxx)
target_compile_features(noexperimentalflag
  PRIVATE
    cxx_std_20)

unset(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API)
