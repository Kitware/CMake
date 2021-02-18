include(RunCMake)

unset(RunCMake_TEST_NO_CLEAN)

run_cmake(MultiCommand)
run_cmake(MissingDetails)
run_cmake(DirectIgnoresDetails)
run_cmake(FirstDetailsWin)
run_cmake(DownloadTwice)
run_cmake(DownloadFile)
run_cmake(VarDefinitions)
run_cmake(GetProperties)
run_cmake(UsesTerminalOverride)
run_cmake(MakeAvailable)
run_cmake(MakeAvailableTwice)
run_cmake(MakeAvailableUndeclared)

run_cmake_with_options(ManualSourceDirectory
  -D "FETCHCONTENT_SOURCE_DIR_WITHPROJECT=${CMAKE_CURRENT_LIST_DIR}/WithProject"
)
run_cmake_with_options(ManualSourceDirectoryMissing
  -D "FETCHCONTENT_SOURCE_DIR_WITHPROJECT=${CMAKE_CURRENT_LIST_DIR}/ADirThatDoesNotExist"
)
# Need to use :STRING to prevent CMake from automatically converting it to an
# absolute path
run_cmake_with_options(ManualSourceDirectoryRelative
  -D "FETCHCONTENT_SOURCE_DIR_WITHPROJECT:STRING=WithProject"
)

function(run_FetchContent_TimeStamps)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/TimeStamps)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  # First run should execute the commands
  run_cmake(TimeStamps)

  # Ensure that the file checks we use in the TimeStampsRerun-check.cmake script
  # will not be defeated by file systems with only one second resolution.
  # The IS_NEWER_THAN check returns TRUE if the timestamps of the two files are
  # the same, which has been observed where filesystems only have one second
  # resolution.
  set(cmpTimeStamp   ${RunCMake_TEST_BINARY_DIR}/cmpTimeStamp.txt)
  set(checkTimeStamp ${RunCMake_TEST_BINARY_DIR}/cmpTimeStampCheck.txt)
  file(TOUCH ${cmpTimeStamp})
  file(TOUCH ${checkTimeStamp})
  if("${cmpTimeStamp}" IS_NEWER_THAN "${checkTimeStamp}")
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E sleep 1.125
      COMMAND_ERROR_IS_FATAL LAST
    )
  endif()

  # Run again with no changes, no commands should re-execute
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake(TimeStampsRerun)
endfunction()
run_FetchContent_TimeStamps()

function(run_FetchContent_DirOverrides)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/DirOverrides-build)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(DirOverrides)

  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_with_options(DirOverridesDisconnected
    -D FETCHCONTENT_FULLY_DISCONNECTED=YES
  )
endfunction()
run_FetchContent_DirOverrides()

set(RunCMake_TEST_OUTPUT_MERGE 1)
run_cmake(PreserveEmptyArgs)
set(RunCMake_TEST_OUTPUT_MERGE 0)

# We need to pass through CMAKE_GENERATOR and CMAKE_MAKE_PROGRAM
# to ensure the test can run on machines where the build tool
# isn't on the PATH. Some build slaves explicitly test with such
# an arrangement (e.g. to test with spaces in the path). We also
# pass through the platform and toolset for completeness, even
# though we don't build anything, just in case this somehow affects
# the way the build tool is invoked.
run_cmake_command(ScriptMode
    ${CMAKE_COMMAND}
    -DCMAKE_GENERATOR=${RunCMake_GENERATOR}
    -DCMAKE_GENERATOR_PLATFORM=${RunCMake_GENERATOR_PLATFORM}
    -DCMAKE_GENERATOR_TOOLSET=${RunCMake_GENERATOR_TOOLSET}
    -DCMAKE_MAKE_PROGRAM=${RunCMake_MAKE_PROGRAM}
    -P ${CMAKE_CURRENT_LIST_DIR}/ScriptMode.cmake
)
