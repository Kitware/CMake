set(config_arg)
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/${CHECK_NAME}.clang-tidy")
  set(config_arg "--config-file=${CMAKE_CURRENT_LIST_DIR}/${CHECK_NAME}.clang-tidy")
endif()

foreach(o out err)
  if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/${CHECK_NAME}-std${o}.txt")
    file(READ "${CMAKE_CURRENT_LIST_DIR}/${CHECK_NAME}-std${o}.txt" expect_std${o})
    string(REGEX REPLACE "\n+$" "" expect_std${o} "${expect_std${o}}")
  else()
    set(expect_std${o} "")
  endif()
endforeach()

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

foreach(o out err)
  string(REGEX REPLACE "\n+$" "" actual_std${o} "${actual_std${o}}")
  if(NOT actual_std${o} STREQUAL expect_std${o})
    string(REPLACE "\n" "\n  " expect_std${o}_formatted "  ${expect_std${o}}")
    string(REPLACE "\n" "\n  " actual_std${o}_formatted "  ${actual_std${o}}")
    string(APPEND RunClangTidy_TEST_FAILED "Expected std${o}:\n${expect_std${o}_formatted}\nActual std${o}:\n${actual_std${o}_formatted}\n")
  endif()
endforeach()

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
