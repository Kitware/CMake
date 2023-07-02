cmake_minimum_required(VERSION 3.24)
project(cxx_modules_export_compile_commands CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_executable(export-compile-commands)
target_sources(export-compile-commands
  PRIVATE
    main.cxx
  PRIVATE
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        importable.cxx)
target_compile_features(export-compile-commands PUBLIC cxx_std_20)

add_test(NAME export-compile-commands COMMAND export-compile-commands)
