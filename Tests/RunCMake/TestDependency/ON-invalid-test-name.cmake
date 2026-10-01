cmake_minimum_required(VERSION 4.3)
set(CMAKE_TEST_BUILD_DEPENDS ON)

project(TestDependencyInvalidTestName C)

enable_testing()

add_custom_target(TestDependencyPrereq
  COMMAND
    "${CMAKE_COMMAND}" -E touch
    "${CMAKE_CURRENT_BINARY_DIR}/TestDependencyPrereq-built.txt"
  BYPRODUCTS
    "${CMAKE_CURRENT_BINARY_DIR}/TestDependencyPrereq-built.txt"
  VERBATIM)

add_test(NAME "Target Build Test Invalid Name"
  COMMAND
    "${CMAKE_COMMAND}" -E true
  BUILD_DEPENDS
    TestDependencyPrereq)
