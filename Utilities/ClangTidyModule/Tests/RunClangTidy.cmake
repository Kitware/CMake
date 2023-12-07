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

file(GLOB header_files RELATIVE "${CMAKE_CURRENT_LIST_DIR}/${CHECK_NAME}" "${CMAKE_CURRENT_LIST_DIR}/${CHECK_NAME}/*")
file(REMOVE_RECURSE "${RunClangTiy_BINARY_DIR}/${CHECK_NAME}")
foreach(header_file IN LISTS header_files)
  if(NOT header_file MATCHES "-fixit\\.h\$")
    file(MAKE_DIRECTORY "${RunClangTidy_BINARY_DIR}/${CHECK_NAME}")
    configure_file("${CMAKE_CURRENT_LIST_DIR}/${CHECK_NAME}/${header_file}" "${RunClangTidy_BINARY_DIR}/${CHECK_NAME}/${header_file}" COPYONLY)
  endif()
endforeach()

set(command
  "${CLANG_TIDY_COMMAND}"
  "--load=${CLANG_TIDY_MODULE}"
  "--checks=-*,${CHECK_NAME}"
  "--fix"
  "--format-style=file"
  "--header-filter=/${CHECK_NAME}/"
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

string(REGEX REPLACE " +\n" "\n" actual_stdout "${actual_stdout}")
string(REGEX REPLACE "\n+$" "" actual_stdout "${actual_stdout}")
if(NOT actual_stdout STREQUAL expect_stdout)
  string(REPLACE "\n" "\n  " expect_stdout_formatted "  ${expect_stdout}")
  string(REPLACE "\n" "\n  " actual_stdout_formatted "  ${actual_stdout}")
  string(APPEND RunClangTidy_TEST_FAILED "Expected stdout:\n${expect_stdout_formatted}\nActual stdout:\n${actual_stdout_formatted}\n")
endif()

function(check_fixit expected fallback_expected actual)
  if(EXISTS "${expected}")
    set(expect_fixit_file "${expected}")
  else()
    set(expect_fixit_file "${fallback_expected}")
  endif()
  file(READ "${expect_fixit_file}" expect_fixit)
  file(READ "${actual}" actual_fixit)
  if(NOT expect_fixit STREQUAL actual_fixit)
    string(REPLACE "\n" "\n  " expect_fixit_formatted "  ${expect_fixit}")
    string(REPLACE "\n" "\n  " actual_fixit_formatted "  ${actual_fixit}")
    string(APPEND RunClangTidy_TEST_FAILED "Expected fixit for ${actual}:\n${expect_fixit_formatted}\nActual fixit:\n${actual_fixit_formatted}\n")
    set(RunClangTidy_TEST_FAILED "${RunClangTidy_TEST_FAILED}" PARENT_SCOPE)
  endif()
endfunction()

check_fixit(
  "${CMAKE_CURRENT_LIST_DIR}/${CHECK_NAME}-fixit.cxx"
  "${CMAKE_CURRENT_LIST_DIR}/${CHECK_NAME}.cxx"
  "${source_file}"
  )

foreach(header_file IN LISTS header_files)
  if(NOT header_file MATCHES "-fixit\\.h\$")
    string(REGEX REPLACE "\\.h\$" "-fixit.h" header_fixit "${header_file}")
    check_fixit(
      "${CMAKE_CURRENT_LIST_DIR}/${CHECK_NAME}/${header_fixit}"
      "${CMAKE_CURRENT_LIST_DIR}/${CHECK_NAME}/${header_file}"
      "${RunClangTidy_BINARY_DIR}/${CHECK_NAME}/${header_file}"
      )
  endif()
endforeach()

if(RunClangTidy_TEST_FAILED)
  string(REPLACE ";" " " command_formatted "${command}")
  message(FATAL_ERROR "Command:\n  ${command_formatted}\n${RunClangTidy_TEST_FAILED}")
endif()
