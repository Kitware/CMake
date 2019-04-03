
cmake_minimum_required(VERSION 3.14)

enable_language (C)

set (GENERATE_CONTENT [[
macro (CHECK_VALUE test_msg value expected)
  if (NOT "${value}" STREQUAL "${expected}")
    string (APPEND RunCMake_TEST_FAILED "${test_msg}: actual result:\n [${value}]\nbut expected:\n [${expected}]\n")
  endif()
endmacro()
]])

add_executable (exec1 empty.c)
add_library (shared1 SHARED empty.c)
add_library (static1 STATIC empty.c)

string (APPEND GENERATE_CONTENT [[

check_value ("TARGET_OUTPUT_NAME executable default" "$<TARGET_OUTPUT_NAME:exec1>" "exec1")
check_value ("TARGET_OUTPUT_NAME shared default" "$<TARGET_OUTPUT_NAME:shared1>" "shared1")
check_value ("TARGET_LINKER_OUTPUT_NAME shared linker default" "$<TARGET_LINKER_OUTPUT_NAME:shared1>" "shared1")
check_value ("TARGET_OUTPUT_NAME static default" "$<TARGET_OUTPUT_NAME:static1>" "static1")
check_value ("TARGET_LINKER_OUTPUT_NAME static linker default" "$<TARGET_LINKER_OUTPUT_NAME:static1>" "static1")
]])
if (CMAKE_C_LINKER_SUPPORTS_PDB)
  string(APPEND GENERATE_CONTENT [[
check_value ("TARGET_PDB_OUTPUT_NAME executable PDB default" "$<TARGET_PDB_OUTPUT_NAME:exec1>" "exec1")
check_value ("TARGET_PDB_OUTPUT_NAME shared PDB default" "$<TARGET_PDB_OUTPUT_NAME:shared1>" "shared1")
]])
endif()


add_executable (exec2 empty.c)
set_property (TARGET exec2 PROPERTY OUTPUT_NAME exec2_custom)
add_library (shared2 SHARED empty.c)
set_property (TARGET shared2 PROPERTY OUTPUT_NAME shared2_custom)
add_library (static2 STATIC empty.c)
set_property (TARGET static2 PROPERTY OUTPUT_NAME static2_custom)

string (APPEND GENERATE_CONTENT [[

check_value ("TARGET_OUTPUT_NAME executable custom" "$<TARGET_OUTPUT_NAME:exec2>" "exec2_custom")
check_value ("TARGET_OUTPUT_NAME shared custom" "$<TARGET_OUTPUT_NAME:shared2>" "shared2_custom")
check_value ("TARGET_LINKER_OUTPUT_NAME shared linker custom" "$<TARGET_LINKER_OUTPUT_NAME:shared2>" "shared2_custom")
check_value ("TARGET_OUTPUT_NAME static custom" "$<TARGET_OUTPUT_NAME:static2>" "static2_custom")
check_value ("TARGET_LINKER_OUTPUT_NAME static linker custom" "$<TARGET_LINKER_OUTPUT_NAME:static2>" "static2_custom")
]])
if (CMAKE_C_LINKER_SUPPORTS_PDB)
  string (APPEND GENERATE_CONTENT [[
check_value ("TARGET_PDB_OUTPUT_NAME executable PDB custom" "$<TARGET_PDB_OUTPUT_NAME:exec2>" "exec2_custom")
check_value ("TARGET_PDB_OUTPUT_NAME shared PDB custom" "$<TARGET_PDB_OUTPUT_NAME:shared2>" "shared2_custom")
  ]])
endif()

add_executable (exec3 empty.c)
set_property (TARGET exec3 PROPERTY RUNTIME_OUTPUT_NAME exec3_runtime)
set_property (TARGET exec3 PROPERTY LIBRARY_OUTPUT_NAME exec3_library)
set_property (TARGET exec3 PROPERTY ARCHIVE_OUTPUT_NAME exec3_archive)
set_property (TARGET exec3 PROPERTY PDB_NAME exec3_pdb)
add_library (shared3 SHARED empty.c)
set_property (TARGET shared3 PROPERTY RUNTIME_OUTPUT_NAME shared3_runtime)
set_property (TARGET shared3 PROPERTY LIBRARY_OUTPUT_NAME shared3_library)
set_property (TARGET shared3 PROPERTY ARCHIVE_OUTPUT_NAME shared3_archive)
set_property (TARGET shared3 PROPERTY PDB_NAME shared3_pdb)
add_library (static3 STATIC empty.c)
set_property (TARGET static3 PROPERTY RUNTIME_OUTPUT_NAME static3_runtime)
set_property (TARGET static3 PROPERTY LIBRARY_OUTPUT_NAME static3_library)
set_property (TARGET static3 PROPERTY ARCHIVE_OUTPUT_NAME static3_archive)
set_property (TARGET static3 PROPERTY PDB_NAME static3_pdb)

string (APPEND GENERATE_CONTENT [[

check_value ("TARGET_OUTPUT_NAME executable all properties" "$<TARGET_OUTPUT_NAME:exec3>" "exec3_runtime")
check_value ("TARGET_OUTPUT_NAME shared all properties" "$<TARGET_OUTPUT_NAME:shared3>" "$<IF:$<IN_LIST:$<PLATFORM_ID>,Windows$<SEMICOLON>CYGWIN>,shared3_runtime,shared3_library>")
check_value ("TARGET_LINKER_OUTPUT_NAME shared linker all properties" "$<TARGET_LINKER_OUTPUT_NAME:shared3>" "$<IF:$<IN_LIST:$<PLATFORM_ID>,Windows$<SEMICOLON>CYGWIN>,shared3_archive,shared3_library>")
check_value ("TARGET_OUTPUT_NAME static all properties" "$<TARGET_OUTPUT_NAME:static3>" "static3_archive")
check_value ("TARGET_LINKER_OUTPUT_NAME static linker all properties" "$<TARGET_LINKER_OUTPUT_NAME:static3>" "static3_archive")
]])
if (CMAKE_C_LINKER_SUPPORTS_PDB)
  string (APPEND GENERATE_CONTENT [[
check_value ("TARGET_PDB_OUTPUT_NAME executable PDB all properties" "$<TARGET_PDB_OUTPUT_NAME:exec3>" "exec3_pdb")
check_value ("TARGET_PDB_OUTPUT_NAME shared PDB all properties" "$<TARGET_PDB_OUTPUT_NAME:shared3>" "shared3_pdb")
]])
endif()


unset(GENERATE_CONDITION)
get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_isMultiConfig)
  list(GET CMAKE_CONFIGURATION_TYPES 0 FIRST_CONFIG)
  set(GENERATE_CONDITION CONDITION $<CONFIG:${FIRST_CONFIG}>)
endif()

file (GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/TARGET_OUTPUT_NAME-generated.cmake"
  CONTENT "${GENERATE_CONTENT}" ${GENERATE_CONDITION})
