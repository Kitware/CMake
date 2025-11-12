cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_export_sepdir CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_subdirectory(subdir)

install(TARGETS export_sepdir
  EXPORT CXXModules
  FILE_SET modules DESTINATION "lib/cxx/miu")
export(EXPORT CXXModules
  NAMESPACE CXXModules::
  FILE "${CMAKE_CURRENT_BINARY_DIR}/export_sepdir-targets.cmake")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/export_sepdir-config.cmake"
  "include(\"\${CMAKE_CURRENT_LIST_DIR}/export_sepdir-targets.cmake\")
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

add_test(NAME export_sepdir_build
  COMMAND
    "${CMAKE_COMMAND}"
    "-Dexpected_dir=${CMAKE_CURRENT_SOURCE_DIR}"
    "-Dexport_sepdir_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    ${generator}
    -S "${CMAKE_CURRENT_SOURCE_DIR}/test"
    -B "${CMAKE_CURRENT_BINARY_DIR}/test")
