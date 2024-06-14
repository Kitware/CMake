cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_export_from_ninja CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_library(export_from_ninja STATIC)
target_sources(export_from_ninja
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
target_compile_features(export_from_ninja PUBLIC cxx_std_20)

add_library(no_modules STATIC no_modules.cxx)

install(TARGETS export_from_ninja no_modules
  EXPORT CXXModules
  FILE_SET modules DESTINATION "lib/cxx/miu")
install(EXPORT CXXModules
  NAMESPACE CXXModules::
  DESTINATION "lib/cmake/export_from_ninja"
  FILE "export_from_ninja-targets.cmake"
  CXX_MODULES_DIRECTORY "export_from_ninja-cxx-modules")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/export_from_ninja-config.cmake"
  "include(\"\${CMAKE_CURRENT_LIST_DIR}/export_from_ninja-targets.cmake\")
set(\${CMAKE_FIND_PACKAGE_NAME}_FOUND 1)
")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/export_from_ninja-config.cmake"
  DESTINATION "lib/cmake/export_from_ninja")
