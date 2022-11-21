add_library(module INTERFACE IMPORTED)
target_sources(module
  INTERFACE
    FILE_SET fs TYPE CXX_MODULES FILES
      sources/module.cxx)
target_compile_features(module
  INTERFACE
    cxx_std_20)
