
cmake_minimum_required(VERSION 3.14)

enable_language (C)

set (win_platforms Windows CYGWIN)

set (GENERATE_CONTENT [[
macro (CHECK_VALUE test_msg value expected)
  if (NOT "${value}" STREQUAL "${expected}")
    string (APPEND RunCMake_TEST_FAILED "${test_msg}: actual result:\n [${value}]\nbut expected:\n [${expected}]\n")
  endif()
endmacro()
]])

add_executable(exec1 IMPORTED)
add_library (shared1 SHARED IMPORTED)
add_library (static1 STATIC IMPORTED)

string (APPEND GENERATE_CONTENT
"\ncheck_value (\"TARGET_FILE_PREFIX executable default\" \"$<TARGET_FILE_PREFIX:exec1>\" \"\")
check_value (\"TARGET_FILE_PREFIX shared default\" \"$<TARGET_FILE_PREFIX:shared1>\" \"${CMAKE_SHARED_LIBRARY_PREFIX}\")
check_value (\"TARGET_LINKER_FILE_PREFIX shared linker default\" \"$<TARGET_LINKER_FILE_PREFIX:shared1>\" \"$<IF:$<IN_LIST:$<PLATFORM_ID>,${win_platforms}>,${CMAKE_IMPORT_LIBRARY_PREFIX},${CMAKE_SHARED_LIBRARY_PREFIX}>\")
check_value (\"TARGET_FILE_PREFIX static default\" \"$<TARGET_FILE_PREFIX:static1>\" \"${CMAKE_STATIC_LIBRARY_PREFIX}\")
check_value (\"TARGET_LINKER_FILE_PREFIX static linker default\" \"$<TARGET_LINKER_FILE_PREFIX:static1>\"  \"${CMAKE_STATIC_LIBRARY_PREFIX}\")\n")


add_executable (exec2 IMPORTED)
set_property (TARGET exec2 PROPERTY PREFIX exec2_prefix)
set_property (TARGET exec2 PROPERTY ENABLE_EXPORTS TRUE)
set_property (TARGET exec2 PROPERTY IMPORT_PREFIX exec2_import_prefix)
add_library (shared2 SHARED IMPORTED)
set_property (TARGET shared2 PROPERTY PREFIX shared2_prefix)
set_property (TARGET shared2 PROPERTY IMPORT_PREFIX shared2_import_prefix)
add_library (static2 STATIC IMPORTED)
set_property (TARGET static2 PROPERTY PREFIX static2_prefix)
set_property (TARGET static2 PROPERTY IMPORT_PREFIX static2_import_prefix)

string (APPEND GENERATE_CONTENT
"\ncheck_value (\"TARGET_FILE_PREFIX executable custom\" \"$<TARGET_FILE_PREFIX:exec2>\" \"exec2_prefix\")
check_value (\"TARGET_LINKER_FILE_PREFIX executable linker custom\" \"$<TARGET_LINKER_FILE_PREFIX:exec2>\" \"$<IF:$<IN_LIST:$<PLATFORM_ID>,${win_platforms};AIX>,exec2_import_prefix,exec2_prefix>\")
check_value (\"TARGET_FILE_PREFIX shared custom\" \"$<TARGET_FILE_PREFIX:shared2>\" \"shared2_prefix\")
check_value (\"TARGET_LINKER_FILE_PREFIX shared linker custom\" \"$<TARGET_LINKER_FILE_PREFIX:shared2>\" \"$<IF:$<IN_LIST:$<PLATFORM_ID>,${win_platforms}>,shared2_import_prefix,shared2_prefix>\")
check_value (\"TARGET_FILE_PREFIX static custom\" \"$<TARGET_FILE_PREFIX:static2>\" \"static2_prefix\")
check_value (\"TARGET_LINKER_FILE_PREFIX static linker custom\" \"$<TARGET_LINKER_FILE_PREFIX:static2>\" \"static2_prefix\")\n")


file (GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/TARGET_FILE_PREFIX-generated.cmake"
  CONTENT "${GENERATE_CONTENT}")
