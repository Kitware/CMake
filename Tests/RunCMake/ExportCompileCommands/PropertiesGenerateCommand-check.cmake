if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/compile_commands.json")
  set(RunCMake_TEST_FAILED "compile_commands.json not generated")
  return()
endif()

file(READ "${RunCMake_TEST_BINARY_DIR}/compile_commands.json" compile_commands)

macro(check_error)
  if(error)
    message(SEND_ERROR "Unexpected error \"${error}\"\nFor: ${compile_commands}")
  endif()
endmacro()

string(JSON num_commands ERROR_VARIABLE error LENGTH "${compile_commands}")
check_error()

# Only one of the targets has the EXPORT_COMPILE_COMMANDS property enabled.
if(NOT num_commands STREQUAL 1)
  message(SEND_ERROR "Expected 1 compile command, got ${num_commands} for ${compile_commands}")
endif()

# Get the compile command generated.
string(JSON result ERROR_VARIABLE error GET "${compile_commands}" 0)
check_error()
string(JSON result ERROR_VARIABLE error GET "${result}" file)
check_error()

# And ensure the correct target is in that compile command.
cmake_path(COMPARE "${result}" EQUAL "${RunCMake_TEST_SOURCE_DIR}/expected_file.c" good)
if(NOT good)
  message(SEND_ERROR "Expected \"${result}\" in \"${RunCMake_TEST_SOURCE_DIR}/expected_file.c\"")
endif()
