include(RunCMake)
include(RunCTest)

set(RunCMake_TEST_TIMEOUT 60)

unset(ENV{CTEST_PARALLEL_LEVEL})
unset(ENV{CTEST_OUTPUT_ON_FAILURE})

run_cmake_command(repeat-opt-bad1
  ${CMAKE_CTEST_COMMAND} --repeat until-pass
  )
run_cmake_command(repeat-opt-bad2
  ${CMAKE_CTEST_COMMAND} --repeat until-pass:foo
  )
run_cmake_command(repeat-opt-bad3
  ${CMAKE_CTEST_COMMAND} --repeat until-fail:2 --repeat-until-fail 2
  )
run_cmake_command(repeat-opt-bad4
  ${CMAKE_CTEST_COMMAND} --repeat-until-fail 2 --repeat until-fail:2
  )
run_cmake_command(repeat-opt-until-pass
  ${CMAKE_CTEST_COMMAND} --repeat until-pass:2
  )
run_cmake_command(repeat-opt-until-fail
  ${CMAKE_CTEST_COMMAND} --repeat until-fail:2
  )
run_cmake_command(repeat-opt-after-timeout
  ${CMAKE_CTEST_COMMAND} --repeat after-timeout:2
  )

run_cmake_command(repeat-until-fail-bad1
  ${CMAKE_CTEST_COMMAND} --repeat-until-fail
  )
run_cmake_command(repeat-until-fail-bad2
  ${CMAKE_CTEST_COMMAND} --repeat-until-fail foo
  )
run_cmake_command(repeat-until-fail-good
  ${CMAKE_CTEST_COMMAND} --repeat-until-fail 2
  )

function(run_repeat_until_pass_tests)
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/repeat-until-pass-build)
  run_cmake(repeat-until-pass-cmake)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(repeat-until-pass-ctest
    ${CMAKE_CTEST_COMMAND} -C Debug --repeat until-pass:3
    )
endfunction()
run_repeat_until_pass_tests()

function(run_repeat_after_timeout_tests)
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/repeat-after-timeout-build)
  run_cmake(repeat-after-timeout-cmake)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(repeat-after-timeout-ctest
    ${CMAKE_CTEST_COMMAND} -C Debug --repeat after-timeout:3
    )
endfunction()
run_repeat_after_timeout_tests()

function(run_repeat_until_fail_tests)
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/repeat-until-fail-build)
  run_cmake(repeat-until-fail-cmake)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(repeat-until-fail-ctest
    ${CMAKE_CTEST_COMMAND} -C Debug ${ARGN}
    )
endfunction()
run_repeat_until_fail_tests(--repeat-until-fail 3)
run_repeat_until_fail_tests(--repeat until-fail:3)

function(run_BadCTestTestfile)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/BadCTestTestfile)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
subdirs()
")

  run_cmake_command(BadCTestTestfile ${CMAKE_CTEST_COMMAND})
endfunction()
run_BadCTestTestfile()

function(run_MergeOutput)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/MergeOutput)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
add_test(MergeOutput \"${CMAKE_COMMAND}\" -P \"${RunCMake_SOURCE_DIR}/MergeOutput.cmake\")
")

  run_cmake_command(MergeOutput ${CMAKE_CTEST_COMMAND} -V)
endfunction()
run_MergeOutput()

function(run_LabelCount)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/LabelCount)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
add_test(test1 \"${CMAKE_COMMAND}\" -E echo \"test1\")
set_tests_properties(test1 PROPERTIES LABELS 'bar')

add_test(test2 \"${CMAKE_COMMAND}\" -E echo \"test2\")
set_tests_properties(test2 PROPERTIES LABELS 'bar')

add_test(test3 \"${CMAKE_COMMAND}\" -E echo \"test3\")
set_tests_properties(test3 PROPERTIES LABELS 'foo')

add_test(test4 \"${CMAKE_COMMAND}\" -E echo \"test4\")
set_tests_properties(test4 PROPERTIES LABELS 'bar')
")

  run_cmake_command(LabelCount ${CMAKE_CTEST_COMMAND} -V)
endfunction()

run_LabelCount()

function(run_RequiredRegexFoundTest)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/RequiredRegexFound)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
add_test(test1 \"${CMAKE_COMMAND}\" -E echo \"test1\")
set_tests_properties(test1 PROPERTIES PASS_REGULAR_EXPRESSION \"foo;test1;bar\")
")

  run_cmake_command(RequiredRegexFound ${CMAKE_CTEST_COMMAND} -V)
endfunction()
run_RequiredRegexFoundTest()

function(run_RequiredRegexNotFoundTest)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/RequiredRegexNotFound)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
add_test(test1 \"${CMAKE_COMMAND}\" -E echo \"test1\")
set_tests_properties(test1 PROPERTIES PASS_REGULAR_EXPRESSION \"foo;toast1;bar\" WILL_FAIL True)
")

  run_cmake_command(RequiredRegexNotFound ${CMAKE_CTEST_COMMAND} -V)
endfunction()
run_RequiredRegexNotFoundTest()

function(run_FailRegexFoundTest)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/FailRegexFound)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
add_test(test1 \"${CMAKE_COMMAND}\" -E echo \"test1\")
set_tests_properties(test1 PROPERTIES FAIL_REGULAR_EXPRESSION \"foo;test1;bar\" WILL_FAIL True)
")

  run_cmake_command(FailRegexFound ${CMAKE_CTEST_COMMAND} -V)
endfunction()
run_FailRegexFoundTest()

function(run_SkipRegexFoundTest)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/SkipRegexFound)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
add_test(test1 \"${CMAKE_COMMAND}\" -E echo \"test1\")
set_tests_properties(test1 PROPERTIES SKIP_REGULAR_EXPRESSION \"test1\")
")

  run_cmake_command(SkipRegexFound ${CMAKE_CTEST_COMMAND} -V)
endfunction()
run_SkipRegexFoundTest()

function(run_SerialFailed)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/SerialFailed)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
add_test(NoSuchCommand no_such_command)
set_tests_properties(NoSuchCommand PROPERTIES RUN_SERIAL ON)
add_test(Echo \"${CMAKE_COMMAND}\" -E echo \"EchoTest\")
")

  run_cmake_command(SerialFailed ${CMAKE_CTEST_COMMAND} -V)
endfunction()
run_SerialFailed()

function(run_TestLoad name load)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/TestLoad)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
  add_test(TestLoad1 \"${CMAKE_COMMAND}\" -E echo \"test of --test-load\")
  add_test(TestLoad2 \"${CMAKE_COMMAND}\" -E echo \"test of --test-load\")
")
  run_cmake_command(${name} ${CMAKE_CTEST_COMMAND} -j2 --test-load ${load})
endfunction()

# Tests for the --test-load feature of ctest
#
# Spoof a load average value to make these tests more reliable.
set(ENV{__CTEST_FAKE_LOAD_AVERAGE_FOR_TESTING} 5)

# Verify that new tests are not started when the load average exceeds
# our threshold and that they then run once the load average drops.
run_TestLoad(test-load-wait 3)

# Verify that warning message is displayed but tests still start when
# an invalid argument is given.
run_TestLoad(test-load-invalid 'two')

# Verify that new tests are started when the load average falls below
# our threshold.
run_TestLoad(test-load-pass 10)

unset(ENV{__CTEST_FAKE_LOAD_AVERAGE_FOR_TESTING})

function(run_TestOutputSize)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/TestOutputSize)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
  add_test(PassingTest \"${CMAKE_COMMAND}\" -E echo PassingTestOutput)
  add_test(FailingTest \"${CMAKE_COMMAND}\" -E no_such_command)
")
  run_cmake_command(TestOutputSize
    ${CMAKE_CTEST_COMMAND} -M Experimental -T Test
                           --no-compress-output
                           --test-output-size-passed 10
                           --test-output-size-failed 12
    )
endfunction()
run_TestOutputSize()

function(run_TestAffinity)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/TestAffinity)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  # Create a test with affinity enabled.  The default PROCESSORS
  # value is 1, so our expected output checks that this is the
  # number of processors in the mask.
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
  add_test(Affinity \"${TEST_AFFINITY}\")
  set_tests_properties(Affinity PROPERTIES PROCESSOR_AFFINITY ON)
")
  # Run ctest with a large parallel level so that the value is
  # not responsible for capping the number of processors available.
  run_cmake_command(TestAffinity ${CMAKE_CTEST_COMMAND} -V -j 64)
endfunction()
if(TEST_AFFINITY)
  run_TestAffinity()
endif()

function(run_TestStdin)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/TestStdin)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
  add_test(TestStdin \"${TEST_PRINT_STDIN}\")
  ")
  run_cmake_command(TestStdin ${CMAKE_CTEST_COMMAND} -V)
endfunction()
run_TestStdin()

function(show_only_json_check_python v)
  if(RunCMake_TEST_FAILED OR NOT PYTHON_EXECUTABLE)
    return()
  endif()
  set(json_file "${RunCMake_TEST_BINARY_DIR}/ctest.json")
  file(WRITE "${json_file}" "${actual_stdout}")
  set(actual_stdout "" PARENT_SCOPE)
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} "${RunCMake_SOURCE_DIR}/show-only_json-v${v}_check.py" "${json_file}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
    )
  if(NOT result EQUAL 0)
    string(REPLACE "\n" "\n  " output "  ${output}")
    set(RunCMake_TEST_FAILED "Unexpected output:\n${output}" PARENT_SCOPE)
  endif()
endfunction()

function(run_ShowOnly)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/ShowOnly)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
    add_test(ShowOnly \"${CMAKE_COMMAND}\" -E echo)
    set_tests_properties(ShowOnly PROPERTIES
      WILL_FAIL true
      RESOURCE_GROUPS \"2,threads:2,gpus:4;gpus:2,threads:4\"
      REQUIRED_FILES RequiredFileDoesNotExist
      _BACKTRACE_TRIPLES \"file1;1;add_test;file0;;\"
      )
    add_test(ShowOnlyNotAvailable NOT_AVAILABLE)
")
  run_cmake_command(show-only_human ${CMAKE_CTEST_COMMAND} --show-only=human)
  run_cmake_command(show-only_bad ${CMAKE_CTEST_COMMAND} --show-only=bad)
  run_cmake_command(show-only_json-v1 ${CMAKE_CTEST_COMMAND} --show-only=json-v1)
endfunction()
run_ShowOnly()

function(run_NoTests)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/NoTests)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "")
  run_cmake_command(no-tests_ignore ${CMAKE_CTEST_COMMAND} --no-tests=ignore)
  run_cmake_command(no-tests_error ${CMAKE_CTEST_COMMAND} --no-tests=error)
  run_cmake_command(no-tests_bad ${CMAKE_CTEST_COMMAND} --no-tests=bad)
  run_cmake_command(no-tests_legacy ${CMAKE_CTEST_COMMAND})
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/NoTestsScript.cmake" "
    set(CTEST_COMMAND \"${CMAKE_CTEST_COMMAND}\")
    set(CTEST_SOURCE_DIRECTORY \"${RunCMake_SOURCE_DIR}\")
    set(CTEST_BINARY_DIRECTORY \"${RunCMake_TEST_BINARY_DIR}\")
    ctest_start(Experimental)
    ctest_test()
")
  run_cmake_command(
    no-tests-script_ignore ${CMAKE_CTEST_COMMAND} --no-tests=ignore
    -S "${RunCMake_TEST_BINARY_DIR}/NoTestsScript.cmake")
  run_cmake_command(
    no-tests-script_error ${CMAKE_CTEST_COMMAND} --no-tests=error
    -S "${RunCMake_TEST_BINARY_DIR}/NoTestsScript.cmake")
  run_cmake_command(
    no-tests-script_legacy ${CMAKE_CTEST_COMMAND}
    -S "${RunCMake_TEST_BINARY_DIR}/NoTestsScript.cmake")
endfunction()
run_NoTests()

# Check the configuration type variable is passed
run_ctest(check-configuration-type)
