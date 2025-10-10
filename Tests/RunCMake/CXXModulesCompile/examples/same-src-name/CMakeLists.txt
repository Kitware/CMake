cmake_minimum_required(VERSION 3.27...3.28)
project(cxx_modules_same_src_name CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_executable(same_src_name)
target_sources(same_src_name
  PRIVATE
    main.cxx
  PRIVATE
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        a/same.cxx
        b/same.cxx
  )
target_compile_features(same_src_name PUBLIC cxx_std_20)

add_test(NAME same_src_name COMMAND same_src_name)
