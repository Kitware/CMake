include(RunCMake)

set(RunCMake_TEST_OPTIONS
    "-DCMAKE_CROSSCOMPILING_EMULATOR=${PSEUDO_EMULATOR}")

run_cmake(CrosscompilingEmulatorProperty)
run_cmake(TryRun)

function(run_AddTest case)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-build)

  run_cmake(${case})
  unset(RunCMake_TEST_OPTIONS)

  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(${case}-build ${CMAKE_COMMAND} --build . --config Debug)
  unset(RunCMake_TEST_OUTPUT_MERGE)

  run_cmake_command(${case}-test ${CMAKE_CTEST_COMMAND} -C Debug -V)
endfunction()

run_AddTest(AddTest)
run_AddTest(AddTest-CMP0158-OLD)
run_AddTest(AddTest-CMP0158-NEW)

function(CustomCommandGenerator_run_and_build case)
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake(${case})
  run_cmake_command(${case}-build ${CMAKE_COMMAND} --build .)
endfunction()

set(RunCMake_TEST_OPTIONS
"-DCMAKE_CROSSCOMPILING_EMULATOR=${PSEUDO_EMULATOR_CUSTOM_COMMAND}")
CustomCommandGenerator_run_and_build(AddCustomCommand)
CustomCommandGenerator_run_and_build(AddCustomTarget)

set(RunCMake_TEST_OPTIONS
"-DCMAKE_CROSSCOMPILING_EMULATOR=${PSEUDO_EMULATOR_CUSTOM_COMMAND_ARG}\;custom_argument")
CustomCommandGenerator_run_and_build(AddCustomCommandWithArg)
CustomCommandGenerator_run_and_build(AddCustomTargetWithArg)
unset(RunCMake_TEST_OPTIONS)

function(run_EnvCrossCompilingEmulator)
  set(ENV{CMAKE_CROSSCOMPILING_EMULATOR} "${PSEUDO_EMULATOR}")
  run_cmake(EnvCrossCompilingEmulator)
  unset(ENV{CMAKE_CROSSCOMPILING_EMULATOR})
endfunction()
run_EnvCrossCompilingEmulator()
