include(GoogleTest)

# Characters special to CMake lists and a mention of TEST that is not a test
# definition must not disturb the line numbers of the test definitions.
set(source "${CMAKE_CURRENT_BINARY_DIR}/source_line.cpp")
file(WRITE "${source}" [=[
// TEST cases follow; int a[2] = {};
#include "source_line.h"

TEST(Suite, First)
{
}
TEST_F(Suite, Second) { int b[1]; }
TEST_P(
  Suite,
  Third)
{}
int x = 0;
TYPED_TEST(Suite, Fourth) {}
]=])

gtest_add_tests(fake_gtest "" "${source}")

foreach(test_line IN ITEMS
    "Suite.First:4"
    "Suite.Second:7"
    "*/Suite.Third/*:8"
    "Suite/*.Fourth:13"
    )
  string(REGEX MATCH "^(.*):([0-9]+)$" _ "${test_line}")
  set(test "${CMAKE_MATCH_1}")
  set(line "${CMAKE_MATCH_2}")
  if(NOT TEST "${test}")
    message(SEND_ERROR "Test \"${test}\" not defined")
    continue()
  endif()
  get_test_property("${test}" DEF_SOURCE_LINE def_source_line)
  if(NOT def_source_line STREQUAL "${source}:${line}")
    message(SEND_ERROR "Test \"${test}\" has DEF_SOURCE_LINE\n"
      "  ${def_source_line}\nbut expected\n  ${source}:${line}")
  endif()
endforeach()
