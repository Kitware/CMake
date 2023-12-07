enable_language(CXX)

if (NOT CMAKE_GENERATOR MATCHES "Visual Studio")
  message(FATAL_ERROR
    "This test requires a 'Visual Studio' generator to be used.")
endif ()

add_library(imported-cxx-modules IMPORTED INTERFACE)
target_sources(imported-cxx-modules
  INTERFACE
    FILE_SET modules TYPE CXX_MODULES
    BASE_DIRS
      "${CMAKE_CURRENT_SOURCE_DIR}/sources"
    FILES
      sources/module-simple.cxx)
set_target_properties(imported-cxx-modules PROPERTIES
  IMPORTED_CONFIGURATIONS DEBUG
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_CXX_MODULES_COMPILE_FEATURES "cxx_std_20"
  INTERFACE_COMPILE_FEATURES "cxx_std_20"
  IMPORTED_CXX_MODULES_DEBUG "simple=${CMAKE_CURRENT_SOURCE_DIR}/sources/module-simple.cxx")

add_executable(vs-use-imported-cxx-modules
  sources/module-simple-use.cxx)
target_link_libraries(vs-use-imported-cxx-modules
  PRIVATE
    imported-cxx-modules)
