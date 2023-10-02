cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_export_interfaces CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_library(export_interfaces_no_properties STATIC)
target_sources(export_interfaces_no_properties
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
target_compile_features(export_interfaces_no_properties PUBLIC cxx_std_20)

add_library(no_modules STATIC no_modules.cxx)

install(TARGETS export_interfaces_no_properties no_modules
  EXPORT CXXModules
  FILE_SET modules DESTINATION "lib/cxx/miu")
install(EXPORT CXXModules
  NAMESPACE CXXModules::
  DESTINATION "lib/cmake/export_interfaces_no_properties"
  FILE "export_interfaces_no_properties-targets.cmake")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/export_interfaces_no_properties-config.cmake"
  "include(\"\${CMAKE_CURRENT_LIST_DIR}/export_interfaces_no_properties-targets.cmake\")
set(\${CMAKE_FIND_PACKAGE_NAME}_FOUND 1)
")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/export_interfaces_no_properties-config.cmake"
  DESTINATION "lib/cmake/export_interfaces_no_properties")

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

add_test(NAME export_interfaces_build
  COMMAND
    "${CMAKE_COMMAND}"
    "-Dexpected_dir=${CMAKE_INSTALL_PREFIX}/lib/cxx/miu"
    "-Dexport_interfaces_no_properties_DIR=${CMAKE_INSTALL_PREFIX}/lib/cmake/export_interfaces_no_properties"
    ${generator}
    -S "${CMAKE_CURRENT_SOURCE_DIR}/test"
    -B "${CMAKE_CURRENT_BINARY_DIR}/test")
