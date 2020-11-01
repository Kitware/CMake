
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

add_executable (exec1 empty.c)
add_library (shared1 SHARED empty.c)
add_library (static1 STATIC empty.c)

string (APPEND GENERATE_CONTENT
"\ncheck_value (\"TARGET_FILE_SUFFIX executable default\" \"$<TARGET_FILE_SUFFIX:exec1>\" \"${CMAKE_EXECUTABLE_SUFFIX}\")
check_value (\"TARGET_FILE_SUFFIX shared default\" \"$<TARGET_FILE_SUFFIX:shared1>\" \"${CMAKE_SHARED_LIBRARY_SUFFIX}\")
check_value (\"TARGET_LINKER_FILE_SUFFIX shared linker default\" \"$<TARGET_LINKER_FILE_SUFFIX:shared1>\" \"$<IF:$<IN_LIST:$<PLATFORM_ID>,${win_platforms}>,${CMAKE_IMPORT_LIBRARY_SUFFIX},${CMAKE_SHARED_LIBRARY_SUFFIX}>\")
check_value (\"TARGET_FILE_SUFFIX static default\" \"$<TARGET_FILE_SUFFIX:static1>\" \"${CMAKE_STATIC_LIBRARY_SUFFIX}\")
check_value (\"TARGET_LINKER_FILE_SUFFIX static linker default\" \"$<TARGET_LINKER_FILE_SUFFIX:static1>\" \"${CMAKE_STATIC_LIBRARY_SUFFIX}\")\n")


add_executable (exec2 empty.c)
set_property (TARGET exec2 PROPERTY SUFFIX exec2_suffix)
set_property (TARGET exec2 PROPERTY ENABLE_EXPORTS TRUE)
set_property (TARGET exec2 PROPERTY IMPORT_SUFFIX exec2_import_suffix)
add_library (shared2 SHARED empty.c)
set_property (TARGET shared2 PROPERTY SUFFIX shared2_suffix)
set_property (TARGET shared2 PROPERTY IMPORT_SUFFIX shared2_import_suffix)
add_library (static2 STATIC empty.c)
set_property (TARGET static2 PROPERTY SUFFIX static2_suffix)
set_property (TARGET static2 PROPERTY IMPORT_SUFFIX static2_import_suffix)

string (APPEND GENERATE_CONTENT
"\ncheck_value (\"TARGET_FILE_SUFFIX executable custom\" \"$<TARGET_FILE_SUFFIX:exec2>\" \"exec2_suffix\")
check_value (\"TARGET_LINKER_FILE_SUFFIX executable linker custom\" \"$<TARGET_LINKER_FILE_SUFFIX:exec2>\" \"$<IF:$<IN_LIST:$<PLATFORM_ID>,${win_platforms};AIX>,exec2_import_suffix,exec2_suffix>\")
check_value (\"TARGET_FILE_SUFFIX shared custom\" \"$<TARGET_FILE_SUFFIX:shared2>\" \"shared2_suffix\")
check_value (\"TARGET_LINKER_FILE_SUFFIX shared linker custom\" \"$<TARGET_LINKER_FILE_SUFFIX:shared2>\" \"$<IF:$<IN_LIST:$<PLATFORM_ID>,${win_platforms}>,shared2_import_suffix,shared2_suffix>\")
check_value (\"TARGET_FILE_SUFFIX static custom\" \"$<TARGET_FILE_SUFFIX:static2>\" \"static2_suffix\")
check_value (\"TARGET_LINKER_FILE_SUFFIX static linker custom\" \"$<TARGET_LINKER_FILE_SUFFIX:static2>\" \"static2_suffix\")\n")


file (GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/TARGET_FILE_SUFFIX-generated.cmake"
  CONTENT "${GENERATE_CONTENT}")
