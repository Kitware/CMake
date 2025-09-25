
include(RunCMake)

macro(run_cmake_target test subtest target)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${test}-${subtest} ${CMAKE_COMMAND} --build . --target ${target} ${VERBOSE})

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
endmacro()

run_cmake(bad_SHELL_usage)

if(RunCMake_GENERATOR MATCHES "Ninja|Makefile|Xcode|Visual Studio")
  # Some environments are excluded because they are not able to honor verbose mode
  if (RunCMake_GENERATOR MATCHES "Xcode|Visual Studio"
      AND NOT CMAKE_C_COMPILER_ID STREQUAL "Intel")
    set(RunCMake_TEST_OUTPUT_MERGE TRUE)
    set(RunCMake_TEST_EXPECT_RESULT ".*")
    set(VERBOSE "--verbose")
  endif()

  run_cmake(ARCHIVER_expansion)

  run_cmake_target(ARCHIVER_expansion ARCHIVER archiver)
  run_cmake_target(ARCHIVER_expansion ARCHIVER_SHELL archiver_shell)
  run_cmake_target(ARCHIVER_expansion ARCHIVER_NESTED archiver_nested)
  run_cmake_target(ARCHIVER_expansion ARCHIVER_NESTED_SHELL archiver_nested_shell)
endif()
