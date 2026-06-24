cmake_minimum_required(VERSION 4.3)
set(CMAKE_TEST_BUILD_DEPENDS ON)

project(TestDependencyInterface C)

enable_testing()

add_executable(TestDependencyInterfaceExe main.c)

# Header-only INTERFACE library is not part of the build system and must be
# filtered out of the test_prep dependencies so that no dead
# "TestDependencyIface.dir/all" prerequisite is generated.
add_library(TestDependencyIface INTERFACE)

add_test(NAME InterfaceTest
  COMMAND
    TestDependencyInterfaceExe
  BUILD_DEPENDS
    TestDependencyIface)
