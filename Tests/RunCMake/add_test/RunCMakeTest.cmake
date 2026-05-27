include(RunCMake)

set(ENV{CTEST_OUTPUT_ON_FAILURE} 1)

function(run_case CASE_NAME)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CMP0110-${CASE_NAME}-build)
  run_cmake(CMP0110-${CASE_NAME})
  # Run ctest on the generated CTestTestfile.cmake.
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(CMP0110-${CASE_NAME}-ctest ${CMAKE_CTEST_COMMAND} -C Debug)
endfunction()

set(cases
  AlphaNumeric
  ValidSpecialChars
  OtherSpecialChars
  EscapedSpecialChars
  Space
  LeadingAndTrailingWhitespace
  Semicolon
  Quote
  BracketArgument
  )

if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  list(APPEND cases FormerInvalidSpecialCharsMC)
else()
  list(APPEND cases FormerInvalidSpecialChars)
endif()

foreach(case IN LISTS cases)
  run_case(WARN-${case})
  run_case(OLD-${case})
  run_case(NEW-${case})
endforeach()

block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/EmptyArgument-build)
  run_cmake(EmptyArgument)
  # Run ctest on the generated CTestTestfile.cmake.
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(EmptyArgument-ctest ${CMAKE_CTEST_COMMAND} -C Debug)
endblock()

set(RunCMake_TEST_OPTIONS "-DCMAKE_TEST_LAUNCHER=${PSEUDO_EMULATOR}")
run_cmake(TestLauncherProperty)
block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/TestLauncher-build)

  run_cmake(TestLauncher)
  unset(RunCMake_TEST_OPTIONS)

  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(TestLauncher-build ${CMAKE_COMMAND} --build . --config Debug)
  unset(RunCMake_TEST_OUTPUT_MERGE)

  run_cmake_command(TestLauncher-test ${CMAKE_CTEST_COMMAND} -C Debug -V)
endblock()
unset(RunCMake_TEST_OPTIONS)

function(run_testdependency_case CASE_NAME EXPECT_PRESENT)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/TestDependency-${CASE_NAME}-build)
  run_cmake(TestDependency-${CASE_NAME})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(TestDependency-${CASE_NAME}-check
    ${CMAKE_COMMAND}
    -DRunCMake_TEST_BINARY_DIR=${RunCMake_TEST_BINARY_DIR}
    -Dexpect_present=${EXPECT_PRESENT}
    -P ${RunCMake_SOURCE_DIR}/TestDependency-check-targets.cmake)
  unset(RunCMake_TEST_NO_CLEAN)
endfunction()

if(RunCMake_GENERATOR MATCHES Ninja)
  block()
    run_testdependency_case(DEFAULT FALSE)
    run_testdependency_case(OFF FALSE)
    run_cmake(TestDependency-ON-invalid-test-name)

    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/TestDependency-ON-build)
    run_testdependency_case(ON TRUE)

    if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
      set(TestDependency_CONFIG Debug)
    else()
      set(TestDependency_CONFIG "")
    endif()
    set(TestDependency_BUILD_CONFIG_ARG)
    if(TestDependency_CONFIG)
      set(TestDependency_BUILD_CONFIG_ARG --config ${TestDependency_CONFIG})
    endif()
    set(RunCMake_TEST_OUTPUT_MERGE 1)
    set(RunCMake_TEST_NO_CLEAN 1)
    run_cmake_command(TestDependency-ON-all
      ${CMAKE_COMMAND} --build . ${TestDependency_BUILD_CONFIG_ARG}
      --target test_prep/all)
    run_cmake_command(TestDependency-ON-build
      ${CMAKE_COMMAND} --build . ${TestDependency_BUILD_CONFIG_ARG}
      --target test_prep/TargetBuildTest)
    run_cmake_command(TestDependency-ON-build-check
      ${CMAKE_COMMAND}
      -DRunCMake_TEST_BINARY_DIR=${RunCMake_TEST_BINARY_DIR}
      -P ${RunCMake_SOURCE_DIR}/TestDependency-build-check.cmake)
    unset(RunCMake_TEST_OUTPUT_MERGE)
    unset(RunCMake_TEST_NO_CLEAN)
  endblock()
endif()
