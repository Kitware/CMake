set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  "25d6f6aa-be65-4692-b44e-87b23e96d4e1")

cmake_minimum_required(VERSION 3.29)
project(cxx_modules_import_std_shared CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_library(import_std_shared SHARED)
target_sources(import_std_shared
  PUBLIC
    FILE_SET use_std TYPE CXX_MODULES FILES
      uses-std.cxx)
target_compile_features(import_std_shared PUBLIC cxx_std_23)

add_executable(main
  main.cxx)
target_link_libraries(main PRIVATE import_std_shared)

install(TARGETS import_std_shared
  EXPORT export
  ARCHIVE DESTINATION "lib"
  RUNTIME DESTINATION "bin"
  FILE_SET use_std DESTINATION "lib/cxx/miu")
export(EXPORT export
  NAMESPACE CXXModules::
  FILE "${CMAKE_CURRENT_BINARY_DIR}/import_std_shared-targets.cmake")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/import_std_shared-config.cmake"
  "include(\"\${CMAKE_CURRENT_LIST_DIR}/import_std_shared-targets.cmake\")
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

add_test(NAME import_std_shared_build
  COMMAND
    "${CMAKE_COMMAND}"
    "-Dexpected_dir=${CMAKE_CURRENT_SOURCE_DIR}"
    "-DCMAKE_PREFIX_PATH=${CMAKE_CURRENT_BINARY_DIR}"
    "-DCMAKE_CXX_STDLIB_MODULES_JSON=${CMAKE_CXX_STDLIB_MODULES_JSON}"
    ${generator}
    -S "${CMAKE_CURRENT_SOURCE_DIR}/test"
    -B "${CMAKE_CURRENT_BINARY_DIR}/test")
