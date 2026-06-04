

enable_language(C)


set (GENERATE_CONTENT [[
macro (CHECK_VALUE test_msg value expected)
  if (NOT "${value}" STREQUAL "${expected}")
    string (APPEND RunCMake_TEST_FAILED "${test_msg}: actual result:\n [${value}]\nbut expected:\n [${expected}]\n")
  endif()
endmacro()

]])

set_property(SOURCE foo.c PROPERTY FOO BAR)

get_property(reference SOURCE foo.c PROPERTY FOO)
string (APPEND GENERATE_CONTENT
  "check_value (\"<SOURCE_PROPERTY:foo.c,FOO>\" \"$<SOURCE_PROPERTY:foo.c,FOO>\" \"${reference}\")\n")

get_property(reference SOURCE foo.c DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY FOO)
string (APPEND GENERATE_CONTENT
  "check_value (\"<SOURCE_PROPERTY:foo.c,DIRECTORY:${CMAKE_CURRENT_SOURCE_DIR},FOO>\" \"$<SOURCE_PROPERTY:foo.c,DIRECTORY:${CMAKE_CURRENT_SOURCE_DIR},FOO>\" \"${reference}\")\n")

get_property(reference SOURCE foo.c PROPERTY VOID)
string (APPEND GENERATE_CONTENT
  "check_value (\"<SOURCE_PROPERTY:foo.c,VOID>\" \"$<SOURCE_PROPERTY:foo.c,VOID>\" \"${reference}\")\n")

get_property(reference SOURCE foo.c DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VOID)
string (APPEND GENERATE_CONTENT
  "check_value (\"<SOURCE_PROPERTY:foo.c,DIRECTORY:${CMAKE_CURRENT_SOURCE_DIR},VOID>\" \"$<SOURCE_PROPERTY:foo.c,DIRECTORY:${CMAKE_CURRENT_SOURCE_DIR},VOID>\" \"${reference}\")\n")


add_library(foo STATIC foo.c)

get_property(reference SOURCE foo.c TARGET_DIRECTORY foo PROPERTY FOO)
string (APPEND GENERATE_CONTENT
  "check_value (\"<SOURCE_PROPERTY:foo.c,TARGET_DIRECTORY:foo,FOO>\" \"$<SOURCE_PROPERTY:foo.c,TARGET_DIRECTORY:foo,FOO>\" \"${reference}\")\n")

get_property(reference SOURCE foo.c TARGET_DIRECTORY foo PROPERTY VOID)
string (APPEND GENERATE_CONTENT
  "check_value (\"<SOURCE_PROPERTY:foo.c,TARGET_DIRECTORY:foo,VOID>\" \"$<SOURCE_PROPERTY:foo.c,TARGET_DIRECTORY:foo,VOID>\" \"${reference}\")\n")


set_property(SOURCE foo.c PROPERTY INCLUDE_DIRECTORIES "$<$<BOOL:ON>:${CMAKE_CURRENT_SOURCE_DIR}>")
string (APPEND GENERATE_CONTENT
  "check_value (\"<SOURCE_PROPERTY:foo.c,TARGET_DIRECTORY:foo,INCLUDE_DIRECTORIES>\" \"$<SOURCE_PROPERTY:foo.c,TARGET_DIRECTORY:foo,INCLUDE_DIRECTORIES>\" \"${CMAKE_CURRENT_SOURCE_DIR}\")\n")

set_property(SOURCE foo.c PROPERTY COMPILE_DEFINITIONS "$<$<BOOL:ON>:SRC_DEF>")
string (APPEND GENERATE_CONTENT
  "check_value (\"<SOURCE_PROPERTY:foo.c,TARGET_DIRECTORY:foo,COMPILE_DEFINITIONS>\" \"$<SOURCE_PROPERTY:foo.c,TARGET_DIRECTORY:foo,COMPILE_DEFINITIONS>\" \"SRC_DEF\")\n")

set_property(SOURCE foo.c PROPERTY COMPILE_OPTIONS "$<$<BOOL:ON>:SRC_OPT>")
string (APPEND GENERATE_CONTENT
  "check_value (\"<SOURCE_PROPERTY:foo.c,TARGET_DIRECTORY:foo,COMPILE_OPTIONS>\" \"$<SOURCE_PROPERTY:foo.c,TARGET_DIRECTORY:foo,COMPILE_OPTIONS>\" \"SRC_OPT\")\n")

if(NOT CMAKE_GENERATOR MATCHES "Xcode|FASTBuild")
  set_property(SOURCE foo.c PROPERTY OBJECT_NAME "$<$<BOOL:ON>:bar>")
  string (APPEND GENERATE_CONTENT
    "check_value (\"<SOURCE_PROPERTY:foo.c,TARGET_DIRECTORY:foo,OBJECT_NAME>\" \"$<SOURCE_PROPERTY:foo.c,TARGET_DIRECTORY:foo,OBJECT_NAME>\" \"bar\")\n")
endif()

file (GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/Local-generated.cmake"
  CONTENT "${GENERATE_CONTENT}")
