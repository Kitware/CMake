cmake_minimum_required(VERSION 3.20)
project(DynamicAnalysisExample C)

include(CTest)

add_executable(use_after_free use_after_free.c)
add_test(NAME use_after_free COMMAND use_after_free)
