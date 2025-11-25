

enable_language(C)


set (GENERATE_CONTENT [[
macro (CHECK_VALUE test_msg value expected)
  if (NOT "${value}" STREQUAL "${expected}")
    string (APPEND RunCMake_TEST_FAILED "${test_msg}: actual result:\n [${value}]\nbut expected:\n [${expected}]\n")
  endif()
endmacro()

]])


add_library(foo STATIC foo.c)

string (APPEND GENERATE_CONTENT
  "check_value (\"<SOURCE_EXISTS:bar.c>\" \"$<SOURCE_EXISTS:bar.c>\" \"0\")\n"
  "check_value (\"<SOURCE_EXISTS:${CMAKE_CURRENT_SOURCE_DIR}/bar.c>\" \"$<SOURCE_EXISTS:${CMAKE_CURRENT_SOURCE_DIR}/bar.c>\" \"0\")\n")

string (APPEND GENERATE_CONTENT
  "check_value (\"<SOURCE_EXISTS:foo.c>\" \"$<SOURCE_EXISTS:foo.c>\" \"1\")\n"
  "check_value (\"<SOURCE_EXISTS:${CMAKE_CURRENT_SOURCE_DIR}/foo.c>\" \"$<SOURCE_EXISTS:${CMAKE_CURRENT_SOURCE_DIR}/foo.c>\" \"1\")\n"
  "check_value (\"<SOURCE_EXISTS:${CMAKE_CURRENT_SOURCE_DIR}/foo.c>\" \"$<SOURCE_EXISTS:${CMAKE_CURRENT_SOURCE_DIR}/foo.c>\" \"1\")\n"
  "check_value (\"<SOURCE_EXISTS:foo.c,DIRECTORY:${CMAKE_CURRENT_SOURCE_DIR}>\" \"$<SOURCE_EXISTS:foo.c,DIRECTORY:${CMAKE_CURRENT_SOURCE_DIR}>\" \"1\")\n"
  "check_value (\"<SOURCE_EXISTS:foo.c,TARGET_DIRECTORY:foo>\" \"$<SOURCE_EXISTS:foo.c,TARGET_DIRECTORY:foo>\" \"1\")\n")


file (GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/Local-generated.cmake"
  CONTENT "${GENERATE_CONTENT}")
