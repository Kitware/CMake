enable_language(C)

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
"\ncheck_value (\"TARGET_IMPORT_FILE shared library\" \"$<TARGET_IMPORT_FILE:shared1>\" \"$<IF:$<IN_LIST:$<PLATFORM_ID>,${platforms_with_import}>,$<TARGET_LINKER_IMPORT_FILE:shared1>,>\")
check_value (\"TARGET_LINKER_FILE shared library\" \"$<TARGET_LINKER_FILE:shared1>\" \"$<IF:$<IN_LIST:$<PLATFORM_ID>,${platforms_with_import}>,$<TARGET_LINKER_IMPORT_FILE:shared1>,$<TARGET_LINKER_LIBRARY_FILE:shared1>>\")
check_value (\"TARGET_IMPORT_FILE static library\" \"$<TARGET_IMPORT_FILE:static1>\" \"\")
check_value (\"TARGET_IMPORT_FILE executable\" \"$<TARGET_IMPORT_FILE:exec1>\" \"\")\n")


set(lib_with_import ${platforms_with_import})
set(exec_with_import ${platforms_with_import})
if (APPLE AND CMAKE_TAPI)
  list(APPEND lib_with_import Darwin)
endif()
if (CMAKE_SYSTEM_NAME STREQUAL "AIX")
  list(APPEND exec_with_import "AIX")
endif()
set(CMAKE_SHARED_LIBRARY_ENABLE_EXPORTS TRUE)
set(CMAKE_EXECUTABLE_ENABLE_EXPORTS TRUE)

add_library (shared2 SHARED empty.c)
add_executable (exec2 empty.c)


string (APPEND GENERATE_CONTENT
"\ncheck_value (\"TARGET_IMPORT_FILE shared library\" \"$<TARGET_IMPORT_FILE:shared2>\" \"$<IF:$<IN_LIST:$<PLATFORM_ID>,${lib_with_import}>,$<TARGET_LINKER_IMPORT_FILE:shared2>,>\")
check_value (\"TARGET_LINKER_FILE shared library\" \"$<TARGET_LINKER_FILE:shared2>\" \"$<IF:$<IN_LIST:$<PLATFORM_ID>,${lib_with_import}>,$<TARGET_LINKER_IMPORT_FILE:shared2>,$<TARGET_LINKER_LIBRARY_FILE:shared2>>\")
check_value (\"TARGET_IMPORT_FILE executable\" \"$<TARGET_IMPORT_FILE:exec2>\" \"$<IF:$<IN_LIST:$<PLATFORM_ID>,${exec_with_import}>,$<TARGET_LINKER_IMPORT_FILE:exec2>,>\")\n")


file (GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/TARGET_IMPORT_FILE-$<CONFIG>-generated.cmake"
  CONTENT "${GENERATE_CONTENT}")
