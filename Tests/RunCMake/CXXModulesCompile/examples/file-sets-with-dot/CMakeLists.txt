cmake_minimum_required(VERSION 3.28)
project(cxx_modules_file_sets_with_dot CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_executable(file_sets_with_dot)
target_sources(file_sets_with_dot
  PRIVATE
    ./main.cxx
  PRIVATE
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        ./importable.cxx)
target_compile_features(file_sets_with_dot PUBLIC cxx_std_20)

add_test(NAME file_sets_with_dot COMMAND file_sets_with_dot)
