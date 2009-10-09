cmake_minimum_required(VERSION 2.8)
project(SmallAndFast)

include(CTest)

add_executable(echoargs echoargs.c)

add_test(test0 echoargs)
add_test(test1 echoargs 1)
add_test(test2 echoargs 1 2)
