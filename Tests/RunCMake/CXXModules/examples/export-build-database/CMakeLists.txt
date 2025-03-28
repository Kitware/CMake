cmake_minimum_required(VERSION 3.29)
project(cxx_modules_export_build_database CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

include("${CMAKE_SOURCE_DIR}/../export-build-database-setup.cmake")

add_compile_options(-Dfrom_compile_options)
add_compile_definitions(-Dfrom_compile_definitions)
include_directories(from_include_directories)

add_library(provide_flags INTERFACE)
target_compile_options(provide_flags
  INTERFACE
    -Ddep_interface_option)
target_compile_definitions(provide_flags
  INTERFACE
    dep_interface_define)
target_include_directories(provide_flags
  INTERFACE
    dep_interface_include)

add_library(export_build_database)
target_sources(export_build_database
  PRIVATE
    lib.cxx
  PUBLIC
    FILE_SET modules
      TYPE CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        importable.cxx)
target_compile_features(export_build_database PUBLIC cxx_std_20)
target_link_libraries(export_build_database
  PRIVATE
    provide_flags)

set_property(SOURCE importable.cxx APPEND
  PROPERTY COMPILE_FLAGS "-Dfrom_source_flag")
set_property(SOURCE importable.cxx APPEND
  PROPERTY COMPILE_OPTIONS "-Dfrom_source_option")
set_property(SOURCE importable.cxx APPEND
  PROPERTY COMPILE_DEFINITIONS "from_source_define")

set_property(TARGET export_build_database APPEND
  PROPERTY COMPILE_FLAGS "-Dfrom_compile_flags")
target_compile_options(export_build_database
  PRIVATE
    -Dtarget_private_option
  INTERFACE
    -Dtarget_interface_option
  PUBLIC
    -Dtarget_public_option)
target_compile_definitions(export_build_database
  PRIVATE
    target_private_define
  INTERFACE
    target_interface_define
  PUBLIC
    target_public_define)
target_include_directories(export_build_database
  PRIVATE
    target_private_include
  INTERFACE
    target_interface_include
  PUBLIC
    target_public_include)

install(TARGETS export_build_database
  EXPORT CXXModules
  FILE_SET modules DESTINATION "lib/cxx/miu")
export(EXPORT CXXModules
  NAMESPACE CXXModules::
  FILE "${CMAKE_CURRENT_BINARY_DIR}/export_build_database-targets.cmake")
install(TARGETS provide_flags
  EXPORT CXXModulesDeps)
export(EXPORT CXXModulesDeps
  NAMESPACE CXXModules::
  FILE "${CMAKE_CURRENT_BINARY_DIR}/export_build_database-dep-targets.cmake")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/export_build_database-config.cmake"
  "include(\"\${CMAKE_CURRENT_LIST_DIR}/export_build_database-dep-targets.cmake\")
include(\"\${CMAKE_CURRENT_LIST_DIR}/export_build_database-targets.cmake\")
set(\${CMAKE_FIND_PACKAGE_NAME}_FOUND 1)
")
