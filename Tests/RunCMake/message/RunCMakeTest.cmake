include(RunCMake)

run_cmake(defaultmessage)
run_cmake(nomessage)
run_cmake(message-internal-warning)
run_cmake(nomessage-internal-warning)
run_cmake(warnmessage)

# Have to explicitly give the command for the working dir to be honoured
set(RunCMake_TEST_COMMAND_WORKING_DIRECTORY /)
run_cmake_command(
    warnmessage-rootdir
    ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/warnmessage-rootdir.cmake
  )
unset(RunCMake_TEST_COMMAND_WORKING_DIRECTORY)

# message command sets fatal occurred flag, so check each type of error

# separately
run_cmake(errormessage_deprecated)
run_cmake(errormessage_dev)

foreach(opt IN ITEMS loglevel log-level)
  run_cmake_command(
      message-${opt}-invalid
      ${CMAKE_COMMAND} --${opt}=blah -P ${RunCMake_SOURCE_DIR}/message-all-loglevels.cmake
    )

  # Checking various combinations of `message(...)` and log levels `WARNING` to `TRACE`
  # - no CLI option -> `WARNING` to `STATUS` output
  run_cmake_command(
      message-${opt}-default
      ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/message-all-loglevels.cmake
    )
  # - Only `WARNING` output
  run_cmake_command(
      message-${opt}-warning
      ${CMAKE_COMMAND} --${opt}=warning -P ${RunCMake_SOURCE_DIR}/message-all-loglevels.cmake
    )
  # - Only `WARNING` and `NOTICE` output
  run_cmake_command(
      message-${opt}-notice
      ${CMAKE_COMMAND} --${opt}=notice -P ${RunCMake_SOURCE_DIR}/message-all-loglevels.cmake
    )
  # - `WARNING` to `STATUS` output
  run_cmake_command(
      message-${opt}-status
      ${CMAKE_COMMAND} --${opt}=status -P ${RunCMake_SOURCE_DIR}/message-all-loglevels.cmake
    )
  # - `WARNING` to `VERBOSE` output
  run_cmake_command(
      message-${opt}-verbose
      ${CMAKE_COMMAND} --${opt}=verbose -P ${RunCMake_SOURCE_DIR}/message-all-loglevels.cmake
    )
  # - `WARNING` to `DEBUG` output
  run_cmake_command(
      message-${opt}-debug
      ${CMAKE_COMMAND} --${opt}=debug -P ${RunCMake_SOURCE_DIR}/message-all-loglevels.cmake
    )
  # - `WARNING` to `TRACE` output
  run_cmake_command(
      message-${opt}-trace
      ${CMAKE_COMMAND} --${opt}=trace -P ${RunCMake_SOURCE_DIR}/message-all-loglevels.cmake
    )
endforeach()

run_cmake_command(
    message-log-level-override
    ${CMAKE_COMMAND} --log-level=debug -DCMAKE_MESSAGE_LOG_LEVEL=TRACE -P ${RunCMake_SOURCE_DIR}/message-all-loglevels.cmake
  )

run_cmake_command(
    message-indent
    ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/message-indent.cmake
  )
run_cmake_command(
    message-indent-multiline
    ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/message-indent-multiline.cmake
  )

run_cmake_command(
    message-context-cli
    ${CMAKE_COMMAND} --log-level=trace --log-context -P ${RunCMake_SOURCE_DIR}/message-context.cmake
  )

run_cmake_command(
    message-context-cache
    ${CMAKE_COMMAND} -DCMAKE_MESSAGE_LOG_LEVEL=TRACE -DCMAKE_MESSAGE_CONTEXT_SHOW=ON -P ${RunCMake_SOURCE_DIR}/message-context.cmake
  )

run_cmake_command(
    message-context-cli-wins-cache
    ${CMAKE_COMMAND} --log-level=verbose --log-context -DCMAKE_MESSAGE_CONTEXT_SHOW=OFF -P ${RunCMake_SOURCE_DIR}/message-context.cmake
  )

run_cmake_command(
    message-checks
    ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/message-checks.cmake
  )
