execute_process(
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/EchoVariableOutput.cmake
  OUTPUT_VARIABLE stdout
  ERROR_QUIET
  ECHO_OUTPUT_VARIABLE
)

file(READ ${CMAKE_CURRENT_LIST_DIR}/EchoVariable-stdout.txt expected_stdout)
if (NOT stdout MATCHES "${expected_stdout}")
  message(FATAL_ERROR "stdout differs from the expected stdout")
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/EchoVariableOutput.cmake
  ERROR_VARIABLE stderr
  OUTPUT_QUIET
  ECHO_ERROR_VARIABLE
)

file(READ ${CMAKE_CURRENT_LIST_DIR}/EchoVariable-stderr.txt expected_stderr)
if (NOT stderr MATCHES "${expected_stderr}")
  message(FATAL_ERROR "stderr differs from the expected stderr")
endif()
