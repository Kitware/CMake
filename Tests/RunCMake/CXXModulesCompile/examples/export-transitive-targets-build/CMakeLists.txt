cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_export_transitive_targets CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_library(usage_reqs INTERFACE
  include/include.h
  includes/includes.h)
target_compile_features(usage_reqs INTERFACE cxx_std_20)
target_compile_definitions(usage_reqs
  INTERFACE
    FROM_USAGE_REQS)
target_include_directories(usage_reqs
  INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/includes>")

add_library(export_transitive_targets STATIC)
target_sources(export_transitive_targets
  PRIVATE
    forward.cxx
  PRIVATE
    FILE_SET modules_private TYPE CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        private.cxx
  PUBLIC
    FILE_SET modules TYPE CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        importable.cxx
        subdir/importable.cxx
  )
target_link_libraries(export_transitive_targets PRIVATE usage_reqs)

add_library(no_modules STATIC no_modules.cxx)

install(TARGETS usage_reqs export_transitive_targets no_modules
  EXPORT CXXModules
  FILE_SET modules DESTINATION "lib/cxx/miu")
export(EXPORT CXXModules
  NAMESPACE CXXModules::
  FILE "${CMAKE_CURRENT_BINARY_DIR}/export_transitive_targets-targets.cmake"
  CXX_MODULES_DIRECTORY "export_transitive_targets-cxx-modules")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/export_transitive_targets-config.cmake"
  "include(\"\${CMAKE_CURRENT_LIST_DIR}/export_transitive_targets-targets.cmake\")
set(\${CMAKE_FIND_PACKAGE_NAME}_FOUND 1)
")

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

add_test(NAME export_transitive_targets_build
  COMMAND
    "${CMAKE_COMMAND}"
    "-Dexpected_source_dir=${CMAKE_CURRENT_SOURCE_DIR}"
    "-Dexpected_binary_dir=${CMAKE_CURRENT_BINARY_DIR}"
    "-Dexport_transitive_targets_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    ${generator}
    -S "${CMAKE_CURRENT_SOURCE_DIR}/test"
    -B "${CMAKE_CURRENT_BINARY_DIR}/test")
