set(config_arg)
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/${CHECK_NAME}.clang-tidy")
  set(config_arg "--config-file=${CMAKE_CURRENT_LIST_DIR}/${CHECK_NAME}.clang-tidy")
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/${CHECK_NAME}-stdout.txt")
  file(READ "${CMAKE_CURRENT_LIST_DIR}/${CHECK_NAME}-stdout.txt" expect_stdout)
  string(REGEX REPLACE "\n+$" "" expect_stdout "${expect_stdout}")
else()
  set(expect_stdout "")
endif()

set(source_file "${RunClangTidy_BINARY_DIR}/${CHECK_NAME}.cxx")
configure_file("${CMAKE_CURRENT_LIST_DIR}/${CHECK_NAME}.cxx" "${source_file}" COPYONLY)

set(command
  "${CLANG_TIDY_COMMAND}"
  "--load=${CLANG_TIDY_MODULE}"
  "--checks=-*,${CHECK_NAME}"
  "--fix"
  "--format-style=file"
  ${config_arg}
  "${source_file}"
  --
  )
execute_process(
  COMMAND ${command}
  RESULT_VARIABLE result
  OUTPUT_VARIABLE actual_stdout
  ERROR_VARIABLE actual_stderr
  )
string(REPLACE "${RunClangTidy_BINARY_DIR}/" "" actual_stdout "${actual_stdout}")

set(RunClangTidy_TEST_FAILED)

if(NOT result EQUAL 0)
  string(APPEND RunClangTidy_TEST_FAILED "Expected result: 0, actual result: ${result}\n")
endif()

string(REGEX REPLACE "\n+$" "" actual_stdout "${actual_stdout}")
if(NOT actual_stdout STREQUAL expect_stdout)
  string(REPLACE "\n" "\n  " expect_stdout_formatted "  ${expect_stdout}")
  string(REPLACE "\n" "\n  " actual_stdout_formatted "  ${actual_stdout}")
  string(APPEND RunClangTidy_TEST_FAILED "Expected stdout:\n${expect_stdout_formatted}\nActual stdout:\n${actual_stdout_formatted}\n")
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/${CHECK_NAME}-fixit.cxx")
  set(expect_fixit_file "${CMAKE_CURRENT_LIST_DIR}/${CHECK_NAME}-fixit.cxx")
else()
  set(expect_fixit_file "${CMAKE_CURRENT_LIST_DIR}/${CHECK_NAME}.cxx")
endif()
file(READ "${expect_fixit_file}" expect_fixit)
file(READ "${source_file}" actual_fixit)
if(NOT expect_fixit STREQUAL actual_fixit)
  string(REPLACE "\n" "\n  " expect_fixit_formatted "  ${expect_fixit}")
  string(REPLACE "\n" "\n  " actual_fixit_formatted "  ${actual_fixit}")
  string(APPEND RunClangTidy_TEST_FAILED "Expected fixit:\n${expect_fixit_formatted}\nActual fixit:\n${actual_fixit_formatted}\n")
endif()

if(RunClangTidy_TEST_FAILED)
  string(REPLACE ";" " " command_formatted "${command}")
  message(FATAL_ERROR "Command:\n  ${command_formatted}\n${RunClangTidy_TEST_FAILED}")
endif()
