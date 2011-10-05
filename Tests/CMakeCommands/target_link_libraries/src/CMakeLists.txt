cmake_minimum_required(VERSION 2.8)

project(src)

if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
