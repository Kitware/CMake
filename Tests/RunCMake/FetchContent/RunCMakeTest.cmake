include(RunCMake)

unset(RunCMake_TEST_NO_CLEAN)

function(run_cmake_with_cmp0168 name)
  run_cmake_with_options("${name}" -D CMP0168=OLD ${ARGN})
  set(RunCMake_TEST_VARIANT_DESCRIPTION "-direct")
  run_cmake_with_options("${name}" -D CMP0168=NEW ${ARGN})
endfunction()

# Won't get to the part where CMP0168 matters
run_cmake_with_options(MissingDetails -D CMP0168=NEW)

# These are testing specific aspects of the sub-build
run_cmake_with_options(SameGenerator -D CMP0168=OLD)
run_cmake_with_options(VarPassthroughs -D CMP0168=OLD)

run_cmake_with_cmp0168(DirectIgnoresDetails)
run_cmake_with_cmp0168(FirstDetailsWin)
block(SCOPE_FOR VARIABLES)
  # Reuse this test to also verify that "cmake --fresh" re-executes the steps
  # when using the direct mode
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/FirstDetailsWin-direct-build)
  set(RunCMake_TEST_VARIANT_DESCRIPTION "-direct-fresh")
  run_cmake_with_options(FirstDetailsWin -D CMP0168=NEW --fresh)
endblock()
run_cmake_with_cmp0168(DownloadTwice)
run_cmake_with_cmp0168(DownloadFile)
run_cmake_with_cmp0168(IgnoreToolchainFile)
run_cmake_with_cmp0168(System)
run_cmake_with_cmp0168(VarDefinitions)
run_cmake_with_cmp0168(GetProperties)
run_cmake_with_cmp0168(UsesTerminalOverride)
run_cmake_with_cmp0168(MakeAvailable)
run_cmake_with_cmp0168(MakeAvailableTwice)
run_cmake_with_cmp0168(MakeAvailableUndeclared)
run_cmake_with_cmp0168(VerifyHeaderSet)

run_cmake_with_cmp0168(FindDependencyExport
  -D "CMAKE_PROJECT_TOP_LEVEL_INCLUDES=${CMAKE_CURRENT_LIST_DIR}/FindDependencyExportDP.cmake"
)

run_cmake_with_cmp0168(ManualSourceDirectory
  -D "FETCHCONTENT_SOURCE_DIR_WITHPROJECT=${CMAKE_CURRENT_LIST_DIR}/WithProject"
)
run_cmake_with_cmp0168(ManualSourceDirectoryMissing
  -D "FETCHCONTENT_SOURCE_DIR_WITHPROJECT=${CMAKE_CURRENT_LIST_DIR}/ADirThatDoesNotExist"
)
# Need to use :STRING to prevent CMake from automatically converting it to an
# absolute path
run_cmake_with_cmp0168(ManualSourceDirectoryRelative
  -D "FETCHCONTENT_SOURCE_DIR_WITHPROJECT:STRING=WithProject"
)

function(run_FetchContent_DirOverrides cmp0168)
  if(cmp0168 STREQUAL "NEW")
    set(RunCMake_TEST_VARIANT_DESCRIPTION "-direct")
  endif()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/DirOverrides${RunCMake_TEST_VARIANT_DESCRIPTION}-build)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake_with_options(DirOverrides -D CMP0168=${cmp0168})

  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_with_options(DirOverridesDisconnected
    -D CMP0168=${cmp0168}
    -D FETCHCONTENT_FULLY_DISCONNECTED=YES
  )
endfunction()
run_FetchContent_DirOverrides(OLD)
run_FetchContent_DirOverrides(NEW)

set(RunCMake_TEST_OUTPUT_MERGE 1)
run_cmake_with_cmp0168(PreserveEmptyArgs)
set(RunCMake_TEST_OUTPUT_MERGE 0)

function(run_FetchContent_ExcludeFromAll)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/ExcludeFromAll-build)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  # We're testing FetchContent_MakeAvailable()'s add_subdirectory() behavior,
  # so it doesn't matter if we use OLD or NEW for CMP0168, but NEW is faster.
  run_cmake(ExcludeFromAll -D CMP0168=NEW)

  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(ExcludeFromAll-build ${CMAKE_COMMAND} --build .)
endfunction()
run_FetchContent_ExcludeFromAll()

# Script mode testing requires more care for CMP0168 set to OLD.
# We need to pass through CMAKE_GENERATOR and CMAKE_MAKE_PROGRAM
# to ensure the test can run on machines where the build tool
# isn't on the PATH. Some build machines explicitly test with such
# an arrangement (e.g. to test with spaces in the path). We also
# pass through the platform and toolset for completeness, even
# though we don't build anything, just in case this somehow affects
# the way the build tool is invoked.
run_cmake_command(ScriptMode
  ${CMAKE_COMMAND}
  -DCMP0168=OLD
  -DCMAKE_GENERATOR=${RunCMake_GENERATOR}
  -DCMAKE_GENERATOR_PLATFORM=${RunCMake_GENERATOR_PLATFORM}
  -DCMAKE_GENERATOR_TOOLSET=${RunCMake_GENERATOR_TOOLSET}
  -DCMAKE_MAKE_PROGRAM=${RunCMake_MAKE_PROGRAM}
  -P ${CMAKE_CURRENT_LIST_DIR}/ScriptMode.cmake
)
# CMP0168 NEW doesn't need a build tool or generator, so don't set them.
run_cmake_command(ScriptMode-direct
  ${CMAKE_COMMAND}
  -DCMP0168=NEW
  -P ${CMAKE_CURRENT_LIST_DIR}/ScriptMode.cmake
)

run_cmake(DisableSourceChanges)
