cmake_minimum_required(VERSION 3.10)
project(CTestTestVerboseOutput)
include(CTest)

add_executable(nop nop.c)

add_test(NAME TestWithProperties COMMAND nop)
set_property(TEST TestWithProperties PROPERTY ENVIRONMENT
  "foo=bar"
  "this=that"
)
