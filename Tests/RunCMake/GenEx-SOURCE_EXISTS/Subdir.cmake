

enable_language(C)


set (GENERATE_CONTENT [[
macro (CHECK_VALUE test_msg value expected)
  if (NOT "${value}" STREQUAL "${expected}")
    string (APPEND RunCMake_TEST_FAILED "${test_msg}: actual result:\n [${value}]\nbut expected:\n [${expected}]\n")
  endif()
endmacro()

]])

add_subdirectory(subdir)

string (APPEND GENERATE_CONTENT
  "check_value (\"<SOURCE_EXISTS:foo.c>\" \"$<SOURCE_EXISTS:foo.c>\" \"0\")\n"
  "check_value (\"<SOURCE_EXISTS:foo.c,DIRECTORY:subdir>\" \"$<SOURCE_EXISTS:foo.c,DIRECTORY:subdir>\" \"1\")\n"
  "check_value (\"<SOURCE_EXISTS:foo.c,TARGET_DIRECTORY:foo>\" \"$<SOURCE_EXISTS:foo.c,TARGET_DIRECTORY:foo>\" \"1\")\n")


file (GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/Subdir-generated.cmake"
  CONTENT "${GENERATE_CONTENT}")
