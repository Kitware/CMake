cmake_minimum_required(VERSION 3.28)
project(cxx_modules_target_objects CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_library(target_objects)
target_sources(target_objects
  PRIVATE
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        importable.cxx)
target_compile_features(target_objects PRIVATE cxx_std_20)

add_executable(main)
target_sources(main
  PRIVATE
    main.cxx
    "$<TARGET_OBJECTS:target_objects>")

add_test(NAME main COMMAND main)
