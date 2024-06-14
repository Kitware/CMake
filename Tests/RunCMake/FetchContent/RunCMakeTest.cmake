include(RunCMake)

unset(RunCMake_TEST_NO_CLEAN)

run_cmake(MissingDetails)
run_cmake(DirectIgnoresDetails)
run_cmake(FirstDetailsWin)
run_cmake(DownloadTwice)
run_cmake(DownloadFile)
run_cmake(IgnoreToolchainFile)
run_cmake(SameGenerator)
run_cmake(System)
run_cmake(VarDefinitions)
run_cmake(VarPassthroughs)
run_cmake(GetProperties)
run_cmake(UsesTerminalOverride)
run_cmake(MakeAvailable)
run_cmake(MakeAvailableTwice)
run_cmake(MakeAvailableUndeclared)
run_cmake(VerifyHeaderSet)

run_cmake_with_options(FindDependencyExport
  -D "CMAKE_PROJECT_TOP_LEVEL_INCLUDES=${CMAKE_CURRENT_LIST_DIR}/FindDependencyExportDP.cmake"
)

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

function(run_FetchContent_ExcludeFromAll)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/ExcludeFromAll-build)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(ExcludeFromAll)

  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(ExcludeFromAll-build ${CMAKE_COMMAND} --build .)
endfunction()
run_FetchContent_ExcludeFromAll()
