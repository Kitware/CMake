cmake_minimum_required(VERSION 3.7)

project(Dependencies)

add_library(myobj OBJECT ${CMAKE_BINARY_DIR}/depends_obj.cpp)
add_library(mylib STATIC $<TARGET_OBJECTS:myobj> depends_lib.cpp)
add_executable(myexe depends_main.cpp)
target_link_libraries(myexe mylib)

enable_testing()
add_test(NAME myexe COMMAND $<TARGET_FILE:myexe>)
