cmake_minimum_required(VERSION 3.4)
project(TestFindLAPACK C)
include(CTest)

find_package(LAPACK REQUIRED)

add_executable(test_tgt main.c)
target_link_libraries(test_tgt LAPACK::LAPACK)
add_test(NAME test_tgt COMMAND test_tgt)

add_executable(test_var main.c)
target_link_libraries(test_var PRIVATE ${LAPACK_LIBRARIES})
add_test(NAME test_var COMMAND test_var)
