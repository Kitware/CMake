cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_export_transitive_modules CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

find_package(export_transitive_modules1 REQUIRED)
find_package(export_transitive_modules1_cps REQUIRED)

add_library(export_transitive_modules STATIC)
target_sources(export_transitive_modules
  PUBLIC
    FILE_SET modules TYPE CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        importable.cxx
  )
target_compile_features(export_transitive_modules PUBLIC cxx_std_20)
target_link_libraries(export_transitive_modules PRIVATE CXXModules::export_transitive_modules1)

add_library(export_transitive_modules_cps STATIC)
target_sources(export_transitive_modules_cps
  PUBLIC
    FILE_SET modules TYPE CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        importable.cxx
  )
target_compile_features(export_transitive_modules_cps PUBLIC cxx_std_20)
target_link_libraries(export_transitive_modules_cps PRIVATE export_transitive_modules1_cps::export_transitive_modules1)
set_target_properties(export_transitive_modules_cps PROPERTIES EXPORT_NAME export_transitive_modules)

install(TARGETS export_transitive_modules
  EXPORT CXXModules
  FILE_SET modules DESTINATION "lib/cxx/miu")
install(TARGETS export_transitive_modules_cps
  EXPORT CXXModules_cps
  FILE_SET modules DESTINATION "lib/cxx/miu")
export(EXPORT CXXModules
  NAMESPACE CXXModules::
  FILE "${CMAKE_CURRENT_BINARY_DIR}/export_transitive_modules-targets.cmake"
  CXX_MODULES_DIRECTORY "export_transitive_modules-cxx-modules")
export(PACKAGE_INFO export_transitive_modules_cps
  EXPORT CXXModules_cps
  CXX_MODULES_DIRECTORY "export_transitive_modules-cxx-modules-cps")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/export_transitive_modules-config.cmake"
  "include(CMakeFindDependencyMacro)
set(export_transitive_modules1_DIR \"${export_transitive_modules1_DIR}\")
find_dependency(export_transitive_modules1)
include(\"\${CMAKE_CURRENT_LIST_DIR}/export_transitive_modules-targets.cmake\")
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

add_test(NAME export_transitive_modules_build
  COMMAND
    "${CMAKE_COMMAND}"
    "-Dexpected_source_dir=${CMAKE_CURRENT_SOURCE_DIR}"
    "-Dexpected_binary_dir=${CMAKE_CURRENT_BINARY_DIR}"
    "-Dexport_transitive_modules_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    "-Dexport_transitive_modules1_cps_DIR=${CMAKE_PREFIX_PATH}"
    "-Dexport_transitive_modules_cps_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    ${generator}
    -S "${CMAKE_CURRENT_SOURCE_DIR}/test"
    -B "${CMAKE_CURRENT_BINARY_DIR}/test")
