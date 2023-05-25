cmake_minimum_required(VERSION 3.24)
project(cxx_modules_export_bmi_and_interfaces CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_library(export_bmi_and_interfaces STATIC)
target_sources(export_bmi_and_interfaces
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
target_compile_features(export_bmi_and_interfaces PUBLIC cxx_std_20)

add_library(no_modules STATIC no_modules.cxx)

install(TARGETS export_bmi_and_interfaces no_modules
  EXPORT CXXModules
  FILE_SET modules DESTINATION "lib/cxx/miu"
  CXX_MODULES_BMI DESTINATION "lib/cxx/bmi")
export(EXPORT CXXModules
  NAMESPACE CXXModules::
  FILE "${CMAKE_CURRENT_BINARY_DIR}/export_bmi_and_interfaces-targets.cmake"
  CXX_MODULES_DIRECTORY "export_bmi_and_interfaces-cxx-modules")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/export_bmi_and_interfaces-config.cmake"
  "include(\"\${CMAKE_CURRENT_LIST_DIR}/export_bmi_and_interfaces-targets.cmake\")
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

add_test(NAME export_bmi_and_interfaces_build
  COMMAND
    "${CMAKE_COMMAND}"
    "-Dexpected_source_dir=${CMAKE_CURRENT_SOURCE_DIR}"
    "-Dexpected_binary_dir=${CMAKE_CURRENT_BINARY_DIR}"
    "-Dexport_bmi_and_interfaces_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    ${generator}
    -S "${CMAKE_CURRENT_SOURCE_DIR}/test"
    -B "${CMAKE_CURRENT_BINARY_DIR}/test")
