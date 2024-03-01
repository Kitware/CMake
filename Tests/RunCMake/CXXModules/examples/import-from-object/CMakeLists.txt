cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_import_from_object CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

set(CMAKE_CXX_STANDARD 20)

add_library(a STATIC)
target_sources(a
  PUBLIC
    FILE_SET CXX_MODULES
      FILES
        object-a.cxx)

add_library(b OBJECT object-b.cxx)
target_link_libraries(b PRIVATE a)
