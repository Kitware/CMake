add_library(module-header SHARED IMPORTED)
target_sources(module-header
  INTERFACE
    FILE_SET fs TYPE CXX_MODULE_HEADER_UNITS FILES
      sources/module-header.h)
target_compile_features(module-header
  INTERFACE
    cxx_std_20)
