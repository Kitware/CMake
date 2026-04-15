function(run_test ACTION INIT TARGET EXPECTED)
  cmake_diagnostic(SET CMD_DEPRECATED ${INIT})

  cmake_diagnostic(GET CMD_DEPRECATED action)
  if(NOT "${action}" STREQUAL "${INIT}")
    message(SEND_ERROR "failed to set diagnostic state")
  endif()

  cmake_diagnostic(${ACTION} CMD_AUTHOR ${TARGET} RECURSE)

  cmake_diagnostic(GET CMD_DEPRECATED action)
  if(NOT "${action}" STREQUAL "${EXPECTED}")
    message(SEND_ERROR
      "failed to change diagnostic state"
      " (expected '${EXPECTED}', actual '${action}')"
    )
  endif()
endfunction()

# Run tests for altering a diagnostic from a known state
run_test(SET IGNORE WARN WARN)
run_test(SET IGNORE SEND_ERROR SEND_ERROR)
run_test(SET IGNORE FATAL_ERROR FATAL_ERROR)

run_test(PROMOTE IGNORE IGNORE IGNORE)
run_test(PROMOTE IGNORE WARN WARN)
run_test(PROMOTE IGNORE SEND_ERROR SEND_ERROR)
run_test(PROMOTE IGNORE FATAL_ERROR FATAL_ERROR)

run_test(PROMOTE WARN IGNORE WARN)
run_test(PROMOTE WARN WARN WARN)
run_test(PROMOTE WARN SEND_ERROR SEND_ERROR)
run_test(PROMOTE WARN FATAL_ERROR FATAL_ERROR)

run_test(PROMOTE SEND_ERROR IGNORE SEND_ERROR)
run_test(PROMOTE SEND_ERROR WARN SEND_ERROR)
run_test(PROMOTE SEND_ERROR SEND_ERROR SEND_ERROR)
run_test(PROMOTE SEND_ERROR FATAL_ERROR FATAL_ERROR)

run_test(PROMOTE FATAL_ERROR IGNORE FATAL_ERROR)
run_test(PROMOTE FATAL_ERROR WARN FATAL_ERROR)
run_test(PROMOTE FATAL_ERROR SEND_ERROR FATAL_ERROR)
run_test(PROMOTE FATAL_ERROR FATAL_ERROR FATAL_ERROR)

run_test(DEMOTE IGNORE IGNORE IGNORE)
run_test(DEMOTE IGNORE WARN IGNORE)
run_test(DEMOTE IGNORE SEND_ERROR IGNORE)
run_test(DEMOTE IGNORE FATAL_ERROR IGNORE)

run_test(DEMOTE WARN IGNORE IGNORE)
run_test(DEMOTE WARN WARN WARN)
run_test(DEMOTE WARN SEND_ERROR WARN)
run_test(DEMOTE WARN FATAL_ERROR WARN)

run_test(DEMOTE SEND_ERROR IGNORE IGNORE)
run_test(DEMOTE SEND_ERROR WARN WARN)
run_test(DEMOTE SEND_ERROR SEND_ERROR SEND_ERROR)
run_test(DEMOTE SEND_ERROR FATAL_ERROR SEND_ERROR)

run_test(DEMOTE FATAL_ERROR IGNORE IGNORE)
run_test(DEMOTE FATAL_ERROR WARN WARN)
run_test(DEMOTE FATAL_ERROR SEND_ERROR SEND_ERROR)
run_test(DEMOTE FATAL_ERROR FATAL_ERROR FATAL_ERROR)

# Ensure that altering a diagnostic that is still in the default state
# uses the default state as the basis for alteration
cmake_diagnostic(DEMOTE CMD_UNINITIALIZED WARN)

cmake_diagnostic(GET CMD_UNINITIALIZED action)
if(NOT "${action}" STREQUAL "IGNORE")
  message(SEND_ERROR
    "CMD_UNINITIALIZED has unexpected state '${action}' (expected 'IGNORE')"
  )
endif()
