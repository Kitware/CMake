execute_process(
  COMMAND echo "Hello from process"
  OUTPUT_VARIABLE output
  ERROR_VARIABLE error
  RESULT_VARIABLE result
  TIMEOUT 5
  WORKING_DIRECTORY /tmp
)
message(STATUS "Output: ${output}")
message(STATUS "Result: ${result}")
