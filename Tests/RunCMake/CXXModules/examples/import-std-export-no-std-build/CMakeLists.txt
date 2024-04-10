set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")

cmake_minimum_required(VERSION 3.29)
project(cxx_modules_import_std_export_no_std CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

set(CMAKE_NO_STD 1)

add_library(import_std_export_no_std)
target_sources(import_std_export_no_std
  PRIVATE
    uses-std.cxx
  PUBLIC
    FILE_SET use_std TYPE CXX_MODULES FILES
      impl-uses-std.cxx)
target_compile_features(import_std_export_no_std PUBLIC cxx_std_23)
set_property(TARGET import_std_export_no_std
  PROPERTY
    CXX_MODULE_STD "$<BOOL:$<BUILD_LOCAL_INTERFACE:1>>")

add_executable(main
  main.cxx)
target_link_libraries(main PRIVATE import_std_export_no_std)

install(TARGETS import_std_export_no_std
  EXPORT export
  ARCHIVE DESTINATION "lib"
  FILE_SET use_std DESTINATION "lib/cxx/miu")
export(EXPORT export
  NAMESPACE CXXModules::
  FILE "${CMAKE_CURRENT_BINARY_DIR}/import_std_export_no_std-targets.cmake")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/import_std_export_no_std-config.cmake"
  "include(\"\${CMAKE_CURRENT_LIST_DIR}/import_std_export_no_std-targets.cmake\")
set(\${CMAKE_FIND_PACKAGE_NAME}_FOUND 1)
")

add_test(NAME main COMMAND main)

set(generator
  -G "${CMAKE_GENERATOR}")
if (CMAKE_GENERATOR_TOOLSET)
  list(APPEND generator
    -T "${CMAKE_GENERATOR_TOOLSET}")
endif ()
if (CMAKE_GENERATOR_PLATFORM)
  list(APPEND generator
    -A "${CMAKE_GENERATOR_PLATFORM}")
endif ()

add_test(NAME import_std_export_no_std_build
  COMMAND
    "${CMAKE_COMMAND}"
    "-Dexpected_dir=${CMAKE_CURRENT_SOURCE_DIR}"
    "-Dimport_std_export_no_std_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    ${generator}
    -S "${CMAKE_CURRENT_SOURCE_DIR}/test"
    -B "${CMAKE_CURRENT_BINARY_DIR}/test")
