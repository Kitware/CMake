enable_language(CXX)

unset(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP)

add_library(noexperimentalflag)
target_sources(noexperimentalflag
  PUBLIC
    FILE_SET fs TYPE CXX_MODULES FILES
      sources/module.cxx)
target_compile_features(noexperimentalflag
  PRIVATE
    cxx_std_20)
