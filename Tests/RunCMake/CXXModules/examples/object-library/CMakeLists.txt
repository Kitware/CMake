cmake_minimum_required(VERSION 3.24)
project(cxx_modules_objlib CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_library(objlib OBJECT)
target_sources(objlib
  PUBLIC
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        importable.cxx)
target_compile_features(objlib PUBLIC cxx_std_20)

add_executable(objmain)
target_sources(objmain
  PRIVATE
    main.cxx)
target_link_libraries(objmain PRIVATE objlib)

add_test(NAME objmain COMMAND objmain)
