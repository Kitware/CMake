cmake_minimum_required(VERSION 4.3)
set(CMAKE_TEST_BUILD_DEPENDS ON)

project(TestDependencyOrphan C)

enable_testing()

# A custom-command output that no target builds.
add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/TestDependencyOrphan-built.txt"
  COMMAND
    "${CMAKE_COMMAND}" -E touch
    "${CMAKE_CURRENT_BINARY_DIR}/TestDependencyOrphan-built.txt"
  VERBATIM)

add_test(NAME OrphanTest
  COMMAND
    "${CMAKE_COMMAND}" -E true
  BUILD_DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/TestDependencyOrphan-built.txt")
