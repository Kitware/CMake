enable_language (C)

set (platforms_with_import Windows CYGWIN MSYS)

set (GENERATE_CONTENT [[
macro (CHECK_VALUE test_msg value expected)
  if (NOT "${value}" STREQUAL "${expected}")
    string (APPEND RunCMake_TEST_FAILED "${test_msg}: actual result:\n [${value}]\nbut expected:\n [${expected}]\n")
  endif()
endmacro()
]])

add_library (shared1 SHARED empty.c)
add_library (static1 STATIC empty.c)
add_executable (exec1 empty.c)

string (APPEND GENERATE_CONTENT
"\ncheck_value (\"TARGET_IMPORT_FILE_SUFFIX executable default\" \"$<TARGET_IMPORT_FILE_SUFFIX:exec1>\" \"\")
check_value (\"TARGET_IMPORT_FILE_SUFFIX shared default\" \"$<TARGET_IMPORT_FILE_SUFFIX:shared1>\" \"$<IF:$<IN_LIST:$<PLATFORM_ID>,${platforms_with_import}>,$<TARGET_LINKER_IMPORT_FILE_SUFFIX:shared1>,>\")
check_value (\"TARGET_FILE_SUFFIX static default\" \"$<TARGET_IMPORT_FILE_SUFFIX:static1>\" \"\")
check_value (\"TARGET_IMPORT_FILE_SUFFIX executable default\" \"$<TARGET_IMPORT_FILE_SUFFIX:exec1>\" \"\")\n")



if (APPLE AND CMAKE_TAPI)
  list(APPEND platforms_with_import Darwin)
endif()
if (CMAKE_SYSTEM_NAME STREQUAL "AIX")
  list(APPEND platforms_with_import AIX)
endif()
set(CMAKE_SHARED_LIBRARY_ENABLE_EXPORTS TRUE)
set(CMAKE_EXECUTABLE_ENABLE_EXPORTS TRUE)

add_library (shared2 SHARED empty.c)
add_executable (exec2 empty.c)

string (APPEND GENERATE_CONTENT
"\ncheck_value (\"TARGET_IMPORT_FILE_SUFFIX executable default\" \"$<TARGET_IMPORT_FILE_SUFFIX:exec2>\" \"$<IF:$<IN_LIST:$<PLATFORM_ID>,${platforms_with_import}>,$<TARGET_LINKER_IMPORT_FILE_SUFFIX:exec2>,>\")
check_value (\"TARGET_IMPORT_FILE_SUFFIX shared default\" \"$<TARGET_IMPORT_FILE_SUFFIX:shared2>\" \"$<IF:$<IN_LIST:$<PLATFORM_ID>,${platforms_with_import}>,$<TARGET_LINKER_IMPORT_FILE_SUFFIX:shared2>,>\")\n")



file (GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/TARGET_IMPORT_FILE_SUFFIX-$<CONFIG>-generated.cmake"
  CONTENT "${GENERATE_CONTENT}")
