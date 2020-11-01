cmake_minimum_required(VERSION 3.15)

set(CMAKE_MACOSX_RPATH OFF)

project(simple-build-test OBJCXX)

add_library(foo SHARED foo.mm)
target_link_libraries(foo "-framework Foundation")

add_executable(simple-build-test main.mm)
target_link_libraries(simple-build-test "-framework Foundation" foo)
