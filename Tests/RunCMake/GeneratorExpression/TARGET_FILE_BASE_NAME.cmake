
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

check_value ("TARGET_FILE_BASE_NAME executable default" "$<TARGET_FILE_BASE_NAME:exec1>" "exec1")
check_value ("TARGET_FILE_BASE_NAME shared default" "$<TARGET_FILE_BASE_NAME:shared1>" "shared1")
check_value ("TARGET_LINKER_FILE_BASE_NAME shared linker default" "$<TARGET_LINKER_FILE_BASE_NAME:shared1>" "shared1")
check_value ("TARGET_FILE_BASE_NAME static default" "$<TARGET_FILE_BASE_NAME:static1>" "static1")
check_value ("TARGET_LINKER_FILE_BASE_NAME static linker default" "$<TARGET_LINKER_FILE_BASE_NAME:static1>" "static1")
]])
if (CMAKE_C_LINKER_SUPPORTS_PDB)
  string(APPEND GENERATE_CONTENT [[
check_value ("TARGET_PDB_FILE_BASE_NAME executable PDB default" "$<TARGET_PDB_FILE_BASE_NAME:exec1>" "exec1")
check_value ("TARGET_PDB_FILE_BASE_NAME shared PDB default" "$<TARGET_PDB_FILE_BASE_NAME:shared1>" "shared1")
]])
endif()


add_executable (exec2 empty.c)
set_property (TARGET exec2 PROPERTY OUTPUT_NAME exec2_custom)
add_library (shared2 SHARED empty.c)
set_property (TARGET shared2 PROPERTY OUTPUT_NAME shared2_custom)
add_library (static2 STATIC empty.c)
set_property (TARGET static2 PROPERTY OUTPUT_NAME static2_custom)

string (APPEND GENERATE_CONTENT [[

check_value ("TARGET_FILE_BASE_NAME executable custom" "$<TARGET_FILE_BASE_NAME:exec2>" "exec2_custom")
check_value ("TARGET_FILE_BASE_NAME shared custom" "$<TARGET_FILE_BASE_NAME:shared2>" "shared2_custom")
check_value ("TARGET_LINKER_FILE_BASE_NAME shared linker custom" "$<TARGET_LINKER_FILE_BASE_NAME:shared2>" "shared2_custom")
check_value ("TARGET_FILE_BASE_NAME static custom" "$<TARGET_FILE_BASE_NAME:static2>" "static2_custom")
check_value ("TARGET_LINKER_FILE_BASE_NAME static linker custom" "$<TARGET_LINKER_FILE_BASE_NAME:static2>" "static2_custom")
]])
if (CMAKE_C_LINKER_SUPPORTS_PDB)
  string (APPEND GENERATE_CONTENT [[
check_value ("TARGET_PDB_FILE_BASE_NAME executable PDB custom" "$<TARGET_PDB_FILE_BASE_NAME:exec2>" "exec2_custom")
check_value ("TARGET_PDB_FILE_BASE_NAME shared PDB custom" "$<TARGET_PDB_FILE_BASE_NAME:shared2>" "shared2_custom")
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

check_value ("TARGET_FILE_BASE_NAME executable all properties" "$<TARGET_FILE_BASE_NAME:exec3>" "exec3_runtime")
check_value ("TARGET_FILE_BASE_NAME shared all properties" "$<TARGET_FILE_BASE_NAME:shared3>" "$<IF:$<IN_LIST:$<PLATFORM_ID>,Windows$<SEMICOLON>CYGWIN>,shared3_runtime,shared3_library>")
check_value ("TARGET_LINKER_FILE_BASE_NAME shared linker all properties" "$<TARGET_LINKER_FILE_BASE_NAME:shared3>" "$<IF:$<IN_LIST:$<PLATFORM_ID>,Windows$<SEMICOLON>CYGWIN>,shared3_archive,shared3_library>")
check_value ("TARGET_FILE_BASE_NAME static all properties" "$<TARGET_FILE_BASE_NAME:static3>" "static3_archive")
check_value ("TARGET_LINKER_FILE_BASE_NAME static linker all properties" "$<TARGET_LINKER_FILE_BASE_NAME:static3>" "static3_archive")
]])
if (CMAKE_C_LINKER_SUPPORTS_PDB)
  string (APPEND GENERATE_CONTENT [[
check_value ("TARGET_PDB_FILE_BASE_NAME executable PDB all properties" "$<TARGET_PDB_FILE_BASE_NAME:exec3>" "exec3_pdb")
check_value ("TARGET_PDB_FILE_BASE_NAME shared PDB all properties" "$<TARGET_PDB_FILE_BASE_NAME:shared3>" "shared3_pdb")
]])
endif()


unset(GENERATE_CONDITION)
get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_isMultiConfig)
  list(GET CMAKE_CONFIGURATION_TYPES 0 FIRST_CONFIG)
  set(GENERATE_CONDITION CONDITION $<CONFIG:${FIRST_CONFIG}>)
else()
  set (FIRST_CONFIG ${CMAKE_BUILD_TYPE})
endif()
string (TOUPPER "${FIRST_CONFIG}" FIRST_CONFIG)


add_executable (exec4 empty.c)
set_property (TARGET exec4 PROPERTY RUNTIME_OUTPUT_NAME exec4_runtime)
set_property (TARGET exec4 PROPERTY LIBRARY_OUTPUT_NAME exec4_library)
set_property (TARGET exec4 PROPERTY ARCHIVE_OUTPUT_NAME exec4_archive)
set_property (TARGET exec4 PROPERTY PDB_NAME exec4_pdb)
set_property (TARGET exec4 PROPERTY ${FIRST_CONFIG}_POSTFIX _postfix)
add_library (shared4 SHARED empty.c)
set_property (TARGET shared4 PROPERTY RUNTIME_OUTPUT_NAME shared4_runtime)
set_property (TARGET shared4 PROPERTY LIBRARY_OUTPUT_NAME shared4_library)
set_property (TARGET shared4 PROPERTY ARCHIVE_OUTPUT_NAME shared4_archive)
set_property (TARGET shared4 PROPERTY PDB_NAME shared4_pdb)
set_property (TARGET shared4 PROPERTY ${FIRST_CONFIG}_POSTFIX _postfix)
add_library (static4 STATIC empty.c)
set_property (TARGET static4 PROPERTY RUNTIME_OUTPUT_NAME static4_runtime)
set_property (TARGET static4 PROPERTY LIBRARY_OUTPUT_NAME static4_library)
set_property (TARGET static4 PROPERTY ARCHIVE_OUTPUT_NAME static4_archive)
set_property (TARGET static4 PROPERTY PDB_NAME static4_pdb)
set_property (TARGET static4 PROPERTY ${FIRST_CONFIG}_POSTFIX _postfix)

string (APPEND GENERATE_CONTENT [[

check_value ("TARGET_FILE_BASE_NAME executable all properties + postfix" "$<TARGET_FILE_BASE_NAME:exec4>" "exec4_runtime_postfix")
check_value ("TARGET_FILE_BASE_NAME shared all properties + postfix" "$<TARGET_FILE_BASE_NAME:shared4>" "$<IF:$<IN_LIST:$<PLATFORM_ID>,Windows$<SEMICOLON>CYGWIN>,shared4_runtime,shared4_library>_postfix")
check_value ("TARGET_LINKER_FILE_BASE_NAME shared linker all properties + postfix" "$<TARGET_LINKER_FILE_BASE_NAME:shared4>" "$<IF:$<IN_LIST:$<PLATFORM_ID>,Windows$<SEMICOLON>CYGWIN>,shared4_archive,shared4_library>_postfix")
check_value ("TARGET_FILE_BASE_NAME static all properties + postfix" "$<TARGET_FILE_BASE_NAME:static4>" "static4_archive_postfix")
check_value ("TARGET_LINKER_FILE_BASE_NAME static linker all properties + postfix" "$<TARGET_LINKER_FILE_BASE_NAME:static4>" "static4_archive_postfix")
]])
if (CMAKE_C_LINKER_SUPPORTS_PDB)
  string (APPEND GENERATE_CONTENT [[
check_value ("TARGET_PDB_FILE_BASE_NAME executable PDB all properties + postfix" "$<TARGET_PDB_FILE_BASE_NAME:exec4>" "exec4_pdb_postfix")
check_value ("TARGET_PDB_FILE_BASE_NAME shared PDB all properties + postfix" "$<TARGET_PDB_FILE_BASE_NAME:shared4>" "shared4_pdb_postfix")
]])
endif()


file (GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/TARGET_FILE_BASE_NAME-generated.cmake"
  CONTENT "${GENERATE_CONTENT}" ${GENERATE_CONDITION})
