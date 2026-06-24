cmake_minimum_required(VERSION 4.3)
set(CMAKE_TEST_BUILD_DEPENDS ON)

project(TestDependencyByproduct C)

enable_testing()

add_executable(TestDependencyByproductExe main.c)
add_custom_command(TARGET TestDependencyByproductExe POST_BUILD
  COMMAND
    "${CMAKE_COMMAND}" -E touch
    "${CMAKE_CURRENT_BINARY_DIR}/TestDependencyByproduct-built.txt"
  BYPRODUCTS
    "${CMAKE_CURRENT_BINARY_DIR}/TestDependencyByproduct-built.txt")

add_test(NAME ByproductTest
  COMMAND
    "${CMAKE_COMMAND}" -E true
  BUILD_DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/TestDependencyByproduct-built.txt")
