cmake_minimum_required(VERSION 3.28)
project(cxx_modules_depchain_modules_json_file CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_library(depchain_with_modules_json_file)
target_sources(depchain_with_modules_json_file
  PUBLIC
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        importable.cxx)
target_compile_features(depchain_with_modules_json_file PUBLIC cxx_std_20)

add_executable(depchain_modules_json_file)
target_sources(depchain_modules_json_file
  PRIVATE
    main.cxx)
target_link_libraries(depchain_modules_json_file
  PRIVATE
    depchain_with_modules_json_file)
add_test(NAME depchain_modules_json_file COMMAND depchain_modules_json_file)
