cmake_minimum_required(VERSION 3.24)
project(cxx_modules_export_usage CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_library(export_usage STATIC)
target_sources(export_usage
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
        importable.cxx)
target_compile_features(export_usage PUBLIC cxx_std_20)

list(APPEND CMAKE_CXX_KNOWN_FEATURES
  exported
  buildiface
  installiface
  buildlocaliface)

target_include_directories(export_usage
  PRIVATE
    "/usr/exported"
    "$<BUILD_INTERFACE:/usr/buildiface>"
    "$<INSTALL_INTERFACE:/usr/installiface>"
    "$<BUILD_LOCAL_INTERFACE:/usr/buildlocaliface>")
target_compile_definitions(export_usage
  PRIVATE
    "exported"
    "$<BUILD_INTERFACE:buildiface>"
    "$<INSTALL_INTERFACE:installiface>"
    "$<BUILD_LOCAL_INTERFACE:buildlocaliface>")
target_compile_features(export_usage
  PRIVATE
    "cxx_std_11"
    "$<BUILD_INTERFACE:cxx_std_14>"
    "$<INSTALL_INTERFACE:cxx_std_17>"
    "$<BUILD_LOCAL_INTERFACE:cxx_std_20>")

if (MSVC)
  set(variable_flag "-constexpr:depth")
else ()
  set(variable_flag "-fconstexpr-depth=")
endif ()

target_compile_options(export_usage
  PRIVATE
    "${variable_flag}100"
    "$<BUILD_INTERFACE:${variable_flag}200>"
    "$<INSTALL_INTERFACE:${variable_flag}300>"
    "$<BUILD_LOCAL_INTERFACE:${variable_flag}400>")

add_library(export_used INTERFACE)
add_library(export_build INTERFACE)
add_library(export_install INTERFACE)
add_library(export_never INTERFACE)

target_link_libraries(export_usage
  PRIVATE
    "export_used"
    "$<BUILD_INTERFACE:export_build>"
    "$<INSTALL_INTERFACE:export_install>"
    "$<BUILD_LOCAL_INTERFACE:export_never>")

install(TARGETS export_usage
  EXPORT CXXModules
  FILE_SET modules DESTINATION "lib/cxx/miu")
install(EXPORT CXXModules
  NAMESPACE CXXModules::
  DESTINATION "lib/cmake/export_usage"
  FILE "export_usage-targets.cmake")
install(TARGETS export_used export_build export_install
  EXPORT CXXModulesDeps)
install(EXPORT CXXModulesDeps
  NAMESPACE CXXModules::
  DESTINATION "lib/cmake/export_usage"
  FILE "export_usage-dep-targets.cmake")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/export_usage-config.cmake"
  "include(\"\${CMAKE_CURRENT_LIST_DIR}/export_usage-dep-targets.cmake\")
include(\"\${CMAKE_CURRENT_LIST_DIR}/export_usage-targets.cmake\")
set(\${CMAKE_FIND_PACKAGE_NAME}_FOUND 1)
")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/export_usage-config.cmake"
  DESTINATION "lib/cmake/export_usage")

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

add_test(NAME export_usage_build
  COMMAND
    "${CMAKE_COMMAND}"
    "-Dexpected_dir=${CMAKE_INSTALL_PREFIX}/lib/cxx/miu"
    "-Dexport_interfaces_flag=${variable_flag}"
    "-Dexport_usage_DIR=${CMAKE_INSTALL_PREFIX}/lib/cmake/export_usage"
    ${generator}
    -S "${CMAKE_CURRENT_SOURCE_DIR}/test"
    -B "${CMAKE_CURRENT_BINARY_DIR}/test")
