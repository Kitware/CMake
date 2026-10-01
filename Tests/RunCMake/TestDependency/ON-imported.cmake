cmake_minimum_required(VERSION 4.3)
set(CMAKE_TEST_BUILD_DEPENDS ON)

project(TestDependencyImported C)

enable_testing()

add_executable(TestDependencyImportedExe ../add_test/main.c)

add_executable(TestDependencyImportedTool IMPORTED)
set_target_properties(TestDependencyImportedTool PROPERTIES
  IMPORTED_LOCATION "${CMAKE_COMMAND}")

add_test(NAME ImportedTest
  COMMAND TestDependencyImportedExe
  BUILD_DEPENDS TestDependencyImportedTool)
