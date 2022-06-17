add_library(module)
target_sources(module
  INTERFACE
    FILE_SET fs TYPE CXX_MODULES FILES
      sources/module.cxx)
target_compile_features(module
  PRIVATE
    cxx_std_20)
