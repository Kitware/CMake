cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_export_transitive_modules1 CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_library(export_transitive_modules1 STATIC)
target_sources(export_transitive_modules1
  PUBLIC
    FILE_SET modules TYPE CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        importable.cxx
  )
target_compile_features(export_transitive_modules1 PUBLIC cxx_std_20)

install(TARGETS export_transitive_modules1
  EXPORT CXXModules
  FILE_SET modules DESTINATION "lib/cxx/miu")
install(EXPORT CXXModules
  NAMESPACE CXXModules::
  DESTINATION "lib/cmake/export_transitive_modules1"
  FILE "export_transitive_modules1-targets.cmake"
  CXX_MODULES_DIRECTORY "export_transitive_modules1-cxx-modules")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/export_transitive_modules1-config.cmake"
  "include(\"\${CMAKE_CURRENT_LIST_DIR}/export_transitive_modules1-targets.cmake\")
set(\${CMAKE_FIND_PACKAGE_NAME}_FOUND 1)
")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/export_transitive_modules1-config.cmake"
  DESTINATION "lib/cmake/export_transitive_modules1")

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

add_test(NAME export_transitive_modules1_build
  COMMAND
    "${CMAKE_COMMAND}"
    "-Dexpected_source_dir=${CMAKE_INSTALL_PREFIX}/lib/cxx/miu"
    "-Dexpected_binary_dir=${CMAKE_INSTALL_PREFIX}/lib/cxx/bmi"
    "-Dexport_transitive_modules1_DIR=${CMAKE_INSTALL_PREFIX}/lib/cmake/export_transitive_modules1"
    ${generator}
    -S "${CMAKE_CURRENT_SOURCE_DIR}/test"
    -B "${CMAKE_CURRENT_BINARY_DIR}/test")
