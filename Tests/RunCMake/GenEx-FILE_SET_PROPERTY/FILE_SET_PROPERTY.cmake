
set (GENERATE_CONTENT [[
macro (CHECK_VALUE test_msg value expected)
  if (NOT "${value}" STREQUAL "${expected}")
    string (APPEND RunCMake_TEST_FAILED "${test_msg}: actual result:\n [${value}]\nbut expected:\n [${expected}]\n")
  endif()
endmacro()

]])

add_library(foo INTERFACE)
target_sources(foo INTERFACE FILE_SET foo TYPE HEADERS FILES file.h)

set_property(FILE_SET foo TARGET foo PROPERTY FOO BAR)

get_property(reference FILE_SET foo TARGET foo PROPERTY FOO)
string (APPEND GENERATE_CONTENT
  "check_value (\"<FILE_SET_PROPERTY:foo,TARGET:foo,FOO>\" \"$<FILE_SET_PROPERTY:foo,TARGET:foo,FOO>\" \"${reference}\")\n")

get_property(reference FILE_SET foo TARGET foo PROPERTY VOID)
string (APPEND GENERATE_CONTENT
  "check_value (\"<FILE_SET_PROPERTY:foo,TARGET:foo,VOID>\" \"$<FILE_SET_PROPERTY:foo,TARGET:foo,VOID>\" \"${reference}\")\n")

file (GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/FILE_SET_PROPERTY-generated.cmake"
  CONTENT "${GENERATE_CONTENT}")
