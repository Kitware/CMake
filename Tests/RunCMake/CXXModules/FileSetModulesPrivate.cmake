enable_language(CXX)
set(CMAKE_EXPERIMENTAL_CXX_SCANDEP_SOURCE "")

add_library(module)
target_sources(module
  PRIVATE
    FILE_SET fs TYPE CXX_MODULES FILES
      sources/module.cxx)
target_compile_features(module
  PRIVATE
    cxx_std_20)
