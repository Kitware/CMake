cmake_minimum_required(VERSION 3.31)
project(cxx_modules_duplicate_sources CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_executable(duplicate_sources)
target_sources(duplicate_sources
  PRIVATE
    main.cxx
  PRIVATE
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        duplicate.cxx
        duplicate.cxx)
target_compile_features(duplicate_sources PRIVATE cxx_std_20)

add_test(NAME duplicate_sources COMMAND duplicate_sources)
