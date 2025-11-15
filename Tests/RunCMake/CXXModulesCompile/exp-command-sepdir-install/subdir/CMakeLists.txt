add_library(export_sepdir STATIC)
target_sources(export_sepdir
  PUBLIC
    FILE_SET modules TYPE CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        importable.cxx)
target_compile_features(export_sepdir PUBLIC cxx_std_20)
