cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_nested_dirs CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_library(export_nested_dirs STATIC)
target_sources(export_nested_dirs
  PUBLIC
    FILE_SET modules TYPE CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        nested/alpha.cxx
        nested/sub/beta.cxx
  )
target_compile_features(export_nested_dirs PUBLIC cxx_std_20)

install(TARGETS export_nested_dirs
  EXPORT CXXModules
  FILE_SET modules DESTINATION "lib/cxx/miu")
export(EXPORT CXXModules
  NAMESPACE CXXModules::
  FILE "${CMAKE_CURRENT_BINARY_DIR}/export_nested_dirs-targets.cmake"
  CXX_MODULES_DIRECTORY "export_nested_dirs-cxx-modules")
export(PACKAGE_INFO export_nested_dirs_cps
  EXPORT CXXModules
  CXX_MODULES_DIRECTORY "export_nested_dirs-cxx-modules-cps")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/export_nested_dirs-config.cmake"
  "include(\"\${CMAKE_CURRENT_LIST_DIR}/export_nested_dirs-targets.cmake\")
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

add_test(NAME export_nested_dirs_build
  COMMAND
    "${CMAKE_COMMAND}"
    "-DCMAKE_PREFIX_PATH=${CMAKE_CURRENT_BINARY_DIR}"
    ${generator}
    -S "${CMAKE_CURRENT_SOURCE_DIR}/test"
    -B "${CMAKE_CURRENT_BINARY_DIR}/test")
