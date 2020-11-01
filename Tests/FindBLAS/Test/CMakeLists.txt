cmake_minimum_required(VERSION 3.4)
project(TestFindBLAS C)
include(CTest)

find_package(BLAS REQUIRED)

add_executable(test_tgt main.c)
target_link_libraries(test_tgt BLAS::BLAS)
add_test(NAME test_tgt COMMAND test_tgt)

add_executable(test_var main.c)
target_link_libraries(test_var PRIVATE ${BLAS_LIBRARIES})
add_test(NAME test_var COMMAND test_var)
