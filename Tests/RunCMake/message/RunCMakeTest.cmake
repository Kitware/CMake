include(RunCMake)

run_cmake(defaultmessage)
run_cmake(nomessage)
run_cmake(message-internal-warning)
run_cmake(nomessage-internal-warning)
run_cmake(warnmessage)
# message command sets fatal occurred flag, so check each type of error

# separately
run_cmake(errormessage_deprecated)
run_cmake(errormessage_dev)

run_cmake_command(
    message-loglevel-invalid
    ${CMAKE_COMMAND} --loglevel=blah -P ${RunCMake_SOURCE_DIR}/message-all-loglevels.cmake
  )

# Checking various combinations of `message(...)` and log levels `WARNING` to `TRACE`
# - no CLI option -> `WARNING` to `STATUS` output
run_cmake_command(
    message-loglevel-default
    ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/message-all-loglevels.cmake
  )
# - Only `WARNING` output
run_cmake_command(
    message-loglevel-warning
    ${CMAKE_COMMAND} --loglevel=warning -P ${RunCMake_SOURCE_DIR}/message-all-loglevels.cmake
  )
# - Only `WARNING` and `NOTICE` output
run_cmake_command(
    message-loglevel-notice
    ${CMAKE_COMMAND} --loglevel=notice -P ${RunCMake_SOURCE_DIR}/message-all-loglevels.cmake
  )
# - `WARNING` to `STATUS` output
run_cmake_command(
    message-loglevel-status
    ${CMAKE_COMMAND} --loglevel=status -P ${RunCMake_SOURCE_DIR}/message-all-loglevels.cmake
  )
# - `WARNING` to `VERBOSE` output
run_cmake_command(
    message-loglevel-verbose
    ${CMAKE_COMMAND} --loglevel=verbose -P ${RunCMake_SOURCE_DIR}/message-all-loglevels.cmake
  )
# - `WARNING` to `DEBUG` output
run_cmake_command(
    message-loglevel-debug
    ${CMAKE_COMMAND} --loglevel=debug -P ${RunCMake_SOURCE_DIR}/message-all-loglevels.cmake
  )
# - `WARNING` to `TRACE` output
run_cmake_command(
    message-loglevel-trace
    ${CMAKE_COMMAND} --loglevel=trace -P ${RunCMake_SOURCE_DIR}/message-all-loglevels.cmake
  )
