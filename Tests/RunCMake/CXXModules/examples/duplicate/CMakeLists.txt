cmake_minimum_required(VERSION 3.24)
project(cxx_modules_duplicate CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_executable(duplicate)
target_sources(duplicate
  PRIVATE
    main.cxx
  PRIVATE
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        duplicate.cxx)
target_compile_features(duplicate PRIVATE cxx_std_20)
target_compile_definitions(duplicate PRIVATE NDUPLICATE=1)

add_executable(duplicate2)
target_sources(duplicate2
  PRIVATE
    main.cxx
  PRIVATE
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        duplicate.cxx)
target_compile_features(duplicate2 PRIVATE cxx_std_20)
target_compile_definitions(duplicate2 PRIVATE NDUPLICATE=2)

add_test(NAME duplicate COMMAND duplicate)
set_property(TEST duplicate
  PROPERTY
    PASS_REGULAR_EXPRESSION "From duplicate #1")
add_test(NAME duplicate2 COMMAND duplicate2)
set_property(TEST duplicate2
  PROPERTY
    PASS_REGULAR_EXPRESSION "From duplicate #2")
