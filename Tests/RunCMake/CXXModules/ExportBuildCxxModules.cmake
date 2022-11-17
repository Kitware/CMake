enable_language(CXX)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
set(CMAKE_EXPERIMENTAL_CXX_SCANDEP_SOURCE "")

add_library(export-modules)
target_sources(export-modules
  PUBLIC
    FILE_SET fs TYPE CXX_MODULES FILES
      sources/module.cxx)
target_compile_features(export-modules
  PRIVATE
    cxx_std_20)
set_property(TARGET export-modules
  PROPERTY EXPORT_NAME export-name)

install(TARGETS export-modules
  EXPORT exp
  FILE_SET fs DESTINATION "include/cxx/export-modules")

export(EXPORT exp
  FILE "${CMAKE_BINARY_DIR}/lib/cmake/export-modules/export-modules-targets.cmake"
  CXX_MODULES_DIRECTORY "cxx-modules")
