cmake_minimum_required(VERSION 3.10)
project(MyProductionCode)

include(CTest)

add_executable(production production.c)
add_test(NAME production COMMAND production)

set_property(TARGET production PROPERTY LABELS MyProductionCode)
set_property(TEST production PROPERTY LABELS MyProductionCode)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY LABELS "NotASubproject")
