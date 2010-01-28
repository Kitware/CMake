cmake_minimum_required (VERSION 2.6)
PROJECT(CrossCompile)

UNSET(run_result CACHE)

#Simulate the cross compile condition
SET(CMAKE_CROSSCOMPILING ON)

ADD_EXECUTABLE(CrossCompile main.c)

TRY_RUN(run_result compile_result
    ${CrossCompile_BINARY_DIR}  
    ${CrossCompile_SOURCE_DIR}/main.c)
