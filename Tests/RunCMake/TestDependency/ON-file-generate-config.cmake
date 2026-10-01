cmake_minimum_required(VERSION 4.3)
set(CMAKE_TEST_BUILD_DEPENDS ON)

project(TestDependencyFileGenerateConfig C)

enable_testing()

add_executable(TestDependencyGenexFileGenerate ../add_test/main.c)

set(rc_file
  "${CMAKE_CURRENT_BINARY_DIR}/gen/TestDependencyGenexFileGenerate/$<CONFIG>/version.rc")
file(GENERATE
  OUTPUT "${rc_file}"
  CONTENT "// Config: $<CONFIG>\n1 RCDATA { \"$<CONFIG>\" }\n"
  TARGET TestDependencyGenexFileGenerate)
target_sources(TestDependencyGenexFileGenerate PRIVATE "${rc_file}")

add_test(NAME FileGenerateConfigTest
  COMMAND $<TARGET_FILE:TestDependencyGenexFileGenerate>)
