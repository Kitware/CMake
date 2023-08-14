enable_language(C)

set (GENERATE_CONTENT [[
macro (CHECK_VALUE test_msg value expected)
  if (NOT "${value}" STREQUAL "${expected}")
    string (APPEND RunCMake_TEST_FAILED "${test_msg}: actual result:\n [${value}]\nbut expected:\n [${expected}]\n")
  endif()
endmacro()
]])

add_library (shared1 SHARED empty.c)
set_property (TARGET shared1 PROPERTY VERSION 2.5.0)
set_property (TARGET shared1 PROPERTY SOVERSION 2.0.0)


string (APPEND GENERATE_CONTENT
"\ncheck_value (\"TARGET_SONAME_IMPORT_FILE shared library\" \"$<TARGET_SONAME_IMPORT_FILE:shared1>\" \"\")\n")



add_library (shared2 SHARED empty.c)
set_property(TARGET shared2 PROPERTY ENABLE_EXPORTS ON)
set_property (TARGET shared2 PROPERTY VERSION 2.5.0)
set_property (TARGET shared2 PROPERTY SOVERSION 2.0.0)


string (APPEND GENERATE_CONTENT
"\ncheck_value (\"TARGET_SONAME_IMPORT_FILE shared library\" \"$<TARGET_SONAME_IMPORT_FILE:shared2>\" \"$<$<BOOL:${CMAKE_TAPI}>:$<PATH:REPLACE_EXTENSION,LAST_ONLY,$<TARGET_SONAME_FILE:shared2>,.tbd>>\")\n")


file (GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/TARGET_SONAME_IMPORT_FILE-$<CONFIG>-generated.cmake"
  CONTENT "${GENERATE_CONTENT}")
