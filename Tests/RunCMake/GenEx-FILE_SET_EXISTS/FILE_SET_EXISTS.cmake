
add_library(foo INTERFACE)
target_sources(foo INTERFACE FILE_SET foo TYPE HEADERS FILES file.h)


set (GENERATE_CONTENT [[
macro (CHECK_VALUE test_msg value expected)
  if (NOT "${value}" STREQUAL "${expected}")
    string (APPEND RunCMake_TEST_FAILED "${test_msg}: actual result:\n [${value}]\nbut expected:\n [${expected}]\n")
  endif()
endmacro()

]])

string(APPEND GENERATE_CONTENT
  "check_value (\"<FILE_SET_EXISTS:bar,TARGET:foo>\" \"$<FILE_SET_EXISTS:bar,TARGET:foo>\" \"0\")\n"
  "check_value (\"<FILE_SET_EXISTS:foo,TARGET:foo>\" \"$<FILE_SET_EXISTS:foo,TARGET:foo>\" \"1\")\n")

file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/FILE_SET_EXISTS-generated.cmake" CONTENT "${GENERATE_CONTENT}")
