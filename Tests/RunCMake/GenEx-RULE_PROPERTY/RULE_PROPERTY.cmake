
add_custom_rule(write_properties OUTPUT "<CURRENT_BINARY_DIR>/RULE_PROPERTY-properties.cmake"
 COMMAND "${CMAKE_COMMAND}" "-DOUTPUT_FILE=<CURRENT_BINARY_DIR>/RULE_PROPERTY-properties.cmake" -P "${CMAKE_CURRENT_SOURCE_DIR}/write_properties.cmake" -- "NAME=$<RULE_PROPERTY:<RULE>,NAME>"
                                                "PARENT_RULE=$<RULE_PROPERTY:<RULE>,PARENT_RULE>"
                                                "GLOBAL=$<RULE_PROPERTY:<RULE>,GLOBAL>"
                                                "VERBATIM=$<RULE_PROPERTY:<RULE>,VERBATIM>"
                                                "OUTPUT_FILE_SET=$<LIST:JOIN,$<RULE_PROPERTY:<RULE>,OUTPUT_FILE_SET>,:>"
                                                "FOO=$<RULE_PROPERTY:<RULE>,FOO>"
                                                "VOID=$<RULE_PROPERTY:<RULE>,VOID>")

set_property(RULE write_properties PROPERTY FOO BAR)


add_custom_target(write_properties ALL)

target_sources(write_properties PRIVATE FILE_SET fs TYPE write_properties FILES foo.txt)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/RULE_PROPERTY-validation.cmake"
  [[
macro (CHECK_VALUE test_msg value expected)
  if (NOT "${value}" STREQUAL "${expected}")
    string (APPEND RunCMake_TEST_FAILED "${test_msg}: actual result:\n [${value}]\nbut expected:\n [${expected}]\n")
  endif()
endmacro()

]])


# predefined properties
set(reference "write_properties")
string (APPEND GENERATE_CONTENT
  "check_value (\"<RULE_PROPERTY:<RULE>,NAME>\" \"\${NAME}\" \"${reference}\")\n")
set(reference "")
string (APPEND GENERATE_CONTENT
  "check_value (\"<RULE_PROPERTY:<RULE>,PARENT_RULE>\" \"\${PARENT_RULE}\" \"${reference}\")\n")
set(reference "0")
string (APPEND GENERATE_CONTENT
  "check_value (\"<RULE_PROPERTY:<RULE>,GLOBAL>\" \"\${GLOBAL}\" \"${reference}\")\n")
set(reference "1")
string (APPEND GENERATE_CONTENT
  "check_value (\"<RULE_PROPERTY:<RULE>,VERBATIM>\" \"\${VERBATIM}\" \"${reference}\")\n")
set(reference "__cmake_rule_write_properties_write_properties_fs_outputs:SOURCES")
string (APPEND GENERATE_CONTENT
  "check_value (\"<RULE_PROPERTY:<RULE>,OUTPUT_FILE_SET>\" \"\${OUTPUT_FILE_SET}\" \"${reference}\")\n")


get_property(reference RULE write_properties PROPERTY FOO)
string (APPEND GENERATE_CONTENT
  "check_value (\"<RULE_PROPERTY:<RULE>,FOO>\" \"\${FOO}\" \"${reference}\")\n")

get_property(reference RULE write_properties PROPERTY VOID)
string (APPEND GENERATE_CONTENT
  "check_value (\"<RULE_PROPERTY:<RULE>,VOID>\" \"\${VOID}\" \"${reference}\")\n")


file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/RULE_PROPERTY-validation.cmake" "${GENERATE_CONTENT}")
