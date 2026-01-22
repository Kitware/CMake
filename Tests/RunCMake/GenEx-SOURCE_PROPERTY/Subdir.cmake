

enable_language(C)


set (GENERATE_CONTENT [[
macro (CHECK_VALUE test_msg value expected)
  if (NOT "${value}" STREQUAL "${expected}")
    string (APPEND RunCMake_TEST_FAILED "${test_msg}: actual result:\n [${value}]\nbut expected:\n [${expected}]\n")
  endif()
endmacro()

]])

set_property(SOURCE foo.c PROPERTY FOO ROOT)

add_subdirectory(subdir)

get_property(reference SOURCE foo.c DIRECTORY "subdir" PROPERTY FOO)
string (APPEND GENERATE_CONTENT
  "check_value (\"<SOURCE_PROPERTY:foo.c,DIRECTORY:subdir,FOO>\" \"$<SOURCE_PROPERTY:foo.c,DIRECTORY:subdir,FOO>\" \"${reference}\")\n")

get_property(reference SOURCE foo.c DIRECTORY "subdir" PROPERTY VOID)
string (APPEND GENERATE_CONTENT
  "check_value (\"<SOURCE_PROPERTY:foo.c,DIRECTORY:subdir,VOID>\" \"$<SOURCE_PROPERTY:foo.c,DIRECTORY:subdir,VOID>\" \"${reference}\")\n")


get_property(reference SOURCE foo.c TARGET_DIRECTORY foo PROPERTY FOO)
string (APPEND GENERATE_CONTENT
  "check_value (\"<SOURCE_PROPERTY:foo.c,TARGET_DIRECTORY:foo,FOO>\" \"$<SOURCE_PROPERTY:foo.c,TARGET_DIRECTORY:foo,FOO>\" \"${reference}\")\n")

get_property(reference SOURCE foo.c TARGET_DIRECTORY foo PROPERTY VOID)
string (APPEND GENERATE_CONTENT
  "check_value (\"<SOURCE_PROPERTY:foo.c,TARGET_DIRECTORY:foo,VOID>\" \"$<SOURCE_PROPERTY:foo.c,TARGET_DIRECTORY:foo,VOID>\" \"${reference}\")\n")


file (GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/Subdir-generated.cmake"
  CONTENT "${GENERATE_CONTENT}")
