cmake_minimum_required(VERSION 3.4)
project(TestFindLZMA C)
include(CTest)

find_package(LibLZMA REQUIRED)

add_executable(test_tgt main.c)
target_link_libraries(test_tgt LibLZMA::LibLZMA)
add_test(NAME test_tgt COMMAND test_tgt)

add_executable(test_var main.c)
target_include_directories(test_var PRIVATE ${LIBLZMA_INCLUDE_DIRS})
target_link_libraries(test_var PRIVATE ${LIBLZMA_LIBRARIES})
add_test(NAME test_var COMMAND test_var)
