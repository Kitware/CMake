cmake_minimum_required(VERSION 3.29)
project(TestFindBLAS C)
include(CTest)

find_package(Backtrace REQUIRED)

add_executable(test_tgt backtrace.c)
target_link_libraries(test_tgt Backtrace::Backtrace)
target_compile_options(test_tgt PUBLIC -rdynamic -fno-omit-frame-pointer)
target_link_options(test_tgt PUBLIC -rdynamic -fno-omit-frame-pointer)
add_test(NAME test_tgt COMMAND test_tgt)
