include(RunCMake)
include(RunCTest)
cmake_policy(SET CMP0140 NEW)

# Do not use any proxy for lookup of an invalid site.
# DNS failure by proxy looks different than DNS failure without proxy.
set(ENV{no_proxy} "$ENV{no_proxy},badhostname.invalid")

set(RunCMake_TEST_TIMEOUT 60)

block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/FailureLabels)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
add_test(ShortName \"${CMAKE_COMMAND}\" -E false)
set_tests_properties(ShortName PROPERTIES LABELS \"Label1;Label2\")
add_test(LongerName \"${CMAKE_COMMAND}\" -E false)
set_tests_properties(LongerName PROPERTIES LABELS \"Label3\")
add_test(Long_Test_Name_That_Is_Over_Fifty_Characters_In_Length \"${CMAKE_COMMAND}\" -E false)
set_tests_properties(Long_Test_Name_That_Is_Over_Fifty_Characters_In_Length PROPERTIES LABELS \"Label4\")
")
  run_cmake_command(FailureLabels ${CMAKE_CTEST_COMMAND})
endblock()

block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/PrintLabels)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
add_test(A \"${CMAKE_COMMAND}\" -E true)
set_tests_properties(A PROPERTIES LABELS \"Label1;Label2\")
add_test(B \"${CMAKE_COMMAND}\" -E true)
set_tests_properties(B PROPERTIES LABELS \"Label3\")
")
  run_cmake_command(PrintLabels ${CMAKE_CTEST_COMMAND} --print-labels)
endblock()

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

function(run_Subdirectories)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/Subdirectories)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}/add_subdirectory")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}/add_subdirectory/sub")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}/subdirs")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}/subdirs/sub")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
add_subdirectory(add_subdirectory)
subdirs(subdirs)
")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/add_subdirectory/CTestTestfile.cmake" "
add_test(add_subdirectory \"${CMAKE_COMMAND}\" -E echo add_subdirectory)
add_subdirectory(sub)
")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/add_subdirectory/sub/CTestTestfile.cmake" "
add_test(add_subdirectory.sub \"${CMAKE_COMMAND}\" -E echo add_subdirectory.sub)
")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/subdirs/CTestTestfile.cmake" "
add_test(subdirs \"${CMAKE_COMMAND}\" -E echo subdirs)
subdirs(sub)
")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/subdirs/sub/CTestTestfile.cmake" "
add_test(subdirs.sub \"${CMAKE_COMMAND}\" -E echo subdirs.sub)
")

  run_cmake_command(Subdirectories ${CMAKE_CTEST_COMMAND} -N)
endfunction()
run_Subdirectories()

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


function(run_TestsFromFileTest case)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/TestsFromFile-${case})
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
add_test(Test1 \"${CMAKE_COMMAND}\" -E echo \"test1\")
add_test(Test2 \"${CMAKE_COMMAND}\" -E echo \"test2\")
add_test(Test11 \"${CMAKE_COMMAND}\" -E echo \"test11\")
")
  run_cmake_command(TestsFromFile-${case} ${CMAKE_CTEST_COMMAND} ${ARGN})
endfunction()
run_TestsFromFileTest(include --tests-from-file ${RunCMake_SOURCE_DIR}/TestsFromFile-TestList.txt)
run_TestsFromFileTest(exclude --exclude-from-file ${RunCMake_SOURCE_DIR}/TestsFromFile-TestList.txt)
run_TestsFromFileTest(include-empty --tests-from-file ${RunCMake_SOURCE_DIR}/TestsFromFile-TestList-empty.txt)
run_TestsFromFileTest(exclude-empty --exclude-from-file ${RunCMake_SOURCE_DIR}/TestsFromFile-TestList-empty.txt)
run_TestsFromFileTest(include-missing --tests-from-file ${RunCMake_SOURCE_DIR}/TestsFromFile-TestList-missing.txt)
run_TestsFromFileTest(exclude-missing --exclude-from-file ${RunCMake_SOURCE_DIR}/TestsFromFile-TestList-missing.txt)


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

function(run_Parallel case)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/Parallel-${case})
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
foreach(i RANGE 1 6)
  add_test(test\${i} \"${CMAKE_COMMAND}\" -E true)
endforeach()
")
  run_cmake_command(Parallel-${case} ${CMAKE_CTEST_COMMAND} ${ARGN})
endfunction()
# Spoof a number of processors to make these tests predictable.
set(ENV{__CTEST_FAKE_PROCESSOR_COUNT_FOR_TESTING} 1)
run_Parallel(bad --parallel bad)
run_Parallel(j-bad -j bad)
set(RunCMake_TEST_RAW_ARGS [[--parallel ""]])
run_Parallel(empty) # With 1 processor, defaults to 2.
unset(RunCMake_TEST_RAW_ARGS)
run_Parallel(j -j) # With 1 processor, defaults to 2.
run_Parallel(0 -j0)
run_Parallel(4 --parallel 4)
run_Parallel(N --parallel -N)
set(ENV{CTEST_PARALLEL_LEVEL} bad)
run_Parallel(env-bad)
if(CMAKE_HOST_WIN32)
  set(ENV{CTEST_PARALLEL_LEVEL} " ")
else()
  set(ENV{CTEST_PARALLEL_LEVEL} "")
endif()
run_Parallel(env-empty) # With 1 processor, defaults to 2.
set(ENV{CTEST_PARALLEL_LEVEL} 0)
run_Parallel(env-0)
set(ENV{CTEST_PARALLEL_LEVEL} 3)
run_Parallel(env-3)
unset(ENV{CTEST_PARALLEL_LEVEL})
unset(ENV{__CTEST_FAKE_PROCESSOR_COUNT_FOR_TESTING)

function(run_TestLoad name load)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/TestLoad)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
  add_test(TestLoad1 \"${CMAKE_COMMAND}\" -E echo \"test of --test-load\")
  set_tests_properties(TestLoad1 PROPERTIES PROCESSORS 2)
  add_test(TestLoad2 \"${CMAKE_COMMAND}\" -E echo \"test of --test-load\")
  set_tests_properties(TestLoad2 PROPERTIES PROCESSORS 2)
")
  run_cmake_command(${name} ${CMAKE_CTEST_COMMAND} -VV -j8 --test-load ${load})
endfunction()

# Tests for the --test-load feature of ctest
#
# Spoof a load average value to make these tests more reliable.
set(ENV{__CTEST_FAKE_LOAD_AVERAGE_FOR_TESTING} 7)

# Verify that new tests are not started when the load average exceeds
# our threshold and that they then run once the load average drops.
run_TestLoad(test-load-wait0 5)
run_TestLoad(test-load-wait1 8)

# Verify that warning message is displayed but tests still start when
# an invalid argument is given.
run_TestLoad(test-load-invalid 'two')

# Verify that new tests are started when the load average falls below
# our threshold.
run_TestLoad(test-load-pass 12)

unset(ENV{__CTEST_FAKE_LOAD_AVERAGE_FOR_TESTING})

function(run_TestOutputSize)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/TestOutputSize)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/DartConfiguration.tcl" "
BuildDirectory: ${RunCMake_TEST_BINARY_DIR}
")
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

# Test --test-output-truncation
function(run_TestOutputTruncation mode expected)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/TestOutputTruncation_${mode})
  set(RunCMake_TEST_NO_CLEAN 1)
  set(TRUNCATED_OUTPUT ${expected})  # used in TestOutputTruncation-check.cmake
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/DartConfiguration.tcl" "
BuildDirectory: ${RunCMake_TEST_BINARY_DIR}
")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
  add_test(Truncation_${mode} \"${CMAKE_COMMAND}\" -E echo 123456789)
")
  run_cmake_command(TestOutputTruncation_${mode}
    ${CMAKE_CTEST_COMMAND} -M Experimental -T Test
                           --no-compress-output
                           --test-output-size-passed 5
                           --test-output-truncation ${mode}
    )
endfunction()
run_TestOutputTruncation("head" "\\.\\.\\.6789")
run_TestOutputTruncation("middle" "12\\.\\.\\..*\\.\\.\\.89")
run_TestOutputTruncation("tail" "12345\\.\\.\\.")
run_TestOutputTruncation("bad" "")

# Test --stop-on-failure
function(run_stop_on_failure)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/stop-on-failure)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
add_test(test1 \"${CMAKE_COMMAND}\" -E false)
add_test(test2 \"${CMAKE_COMMAND}\" -E echo \"not running\")
")
  run_cmake_command(stop-on-failure ${CMAKE_CTEST_COMMAND} --stop-on-failure)
endfunction()
run_stop_on_failure()

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
  if(RunCMake_TEST_FAILED OR NOT Python_EXECUTABLE)
    return()
  endif()
  set(json_file "${RunCMake_TEST_BINARY_DIR}/ctest.json")
  file(WRITE "${json_file}" "${actual_stdout}")
  set(actual_stdout "" PARENT_SCOPE)

  if(CMake_TEST_JSON_SCHEMA)
    execute_process(
      COMMAND ${Python_EXECUTABLE} "${RunCMake_SOURCE_DIR}/show-only_json_validate_schema.py" "${json_file}"
      RESULT_VARIABLE result
      OUTPUT_VARIABLE output
      ERROR_VARIABLE output
    )
    if(NOT result STREQUAL 0)
      string(REPLACE "\n" "\n  " output "${output}")
      string(APPEND RunCMake_TEST_FAILED "Failed to validate version ${v} JSON schema for file: ${file}\nOutput:\n${output}\n")
    endif()
  endif()

  execute_process(
    COMMAND ${Python_EXECUTABLE} "${RunCMake_SOURCE_DIR}/show-only_json-v${v}_check.py" "${json_file}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
    )
  if(NOT result EQUAL 0)
    string(REPLACE "\n" "\n  " output "  ${output}")
    string(APPEND RunCMake_TEST_FAILED "Unexpected output:\n${output}" PARENT_SCOPE)
  endif()
  return(PROPAGATE RunCMake_TEST_FAILED)
endfunction()

function(run_ShowOnly)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/ShowOnly)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
    add_test(ShowOnly \"${CMAKE_COMMAND}\" -E echo)
    set_tests_properties(ShowOnly PROPERTIES
      GENERATED_RESOURCE_SPEC_FILE \"/Path/Does/Not/Exist\"
      RESOURCE_GROUPS \"2,threads:2,gpus:4;gpus:2,threads:4\"
      REQUIRED_FILES RequiredFileDoesNotExist
      _BACKTRACE_TRIPLES \"file1;1;add_test;file0;;\"
      TIMEOUT 1234.5
      TIMEOUT_SIGNAL_NAME SIGINT
      TIMEOUT_SIGNAL_GRACE_PERIOD 2.1
      WILL_FAIL true
      USER_DEFINED_A \"User defined property A value\"
      USER_DEFINED_B \"User defined property B value\"
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

  run_cmake_command(no-tests_env_ignore ${CMAKE_COMMAND} -E env CTEST_NO_TESTS_ACTION=ignore ${CMAKE_CTEST_COMMAND})
  run_cmake_command(no-tests_env_error ${CMAKE_COMMAND} -E env CTEST_NO_TESTS_ACTION=error ${CMAKE_CTEST_COMMAND})
  run_cmake_command(no-tests_env_bad ${CMAKE_COMMAND} -E env CTEST_NO_TESTS_ACTION=bad ${CMAKE_CTEST_COMMAND})
  run_cmake_command(no-tests_env_empty_legacy ${CMAKE_COMMAND} -E env CTEST_NO_TESTS_ACTION= ${CMAKE_CTEST_COMMAND})

  run_cmake_command(no-tests_env_bad_with_cli_error ${CMAKE_COMMAND} -E env CTEST_NO_TESTS_ACTION=bad ${CMAKE_CTEST_COMMAND} --no-tests=error)

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

function(run_FailDrop case)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/FailDrop-${case}-build)
  run_cmake_with_options(FailDrop-${case} ${ARGN})
  unset(ENV{CMAKE_TLS_VERIFY}) # Test that env variable is saved in ctest config file.
  unset(ENV{CMAKE_TLS_VERSION}) # Test that env variable is saved in ctest config file.
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(FailDrop-${case}-ctest
    ${CMAKE_CTEST_COMMAND} -M Experimental -T Submit -VV
    )
endfunction()
run_FailDrop(TLSVersion-1.1 -DCTEST_TLS_VERSION=1.1)
run_FailDrop(TLSVersion-1.1-cmake -DCMAKE_TLS_VERSION=1.1) # Test fallback to CMake variable.
set(ENV{CMAKE_TLS_VERSION} 1.1) # Test fallback to env variable.
run_FailDrop(TLSVersion-1.1-env)
unset(ENV{CMAKE_TLS_VERSION})
run_FailDrop(TLSVerify-ON -DCTEST_TLS_VERIFY=ON)
run_FailDrop(TLSVerify-ON-cmake -DCMAKE_TLS_VERIFY=ON) # Test fallback to CMake variable.
set(ENV{CMAKE_TLS_VERIFY} 1) # Test fallback to env variable.
run_FailDrop(TLSVerify-ON-env)
unset(ENV{CMAKE_TLS_VERIFY})
run_FailDrop(TLSVerify-OFF -DCTEST_TLS_VERIFY=OFF)
run_FailDrop(TLSVerify-OFF-cmake -DCMAKE_TLS_VERIFY=OFF) # Test fallback to CMake variable.
set(ENV{CMAKE_TLS_VERIFY} 0) # Test fallback to env variable.
run_FailDrop(TLSVerify-OFF-env)
unset(ENV{CMAKE_TLS_VERIFY})

run_cmake_command(EmptyDirTest-ctest
  ${CMAKE_CTEST_COMMAND} -C Debug -M Experimental -T Test
  )

run_cmake_command(EmptyDirCoverage-ctest
  # Isolate this test from any surrounding coverage tool.
  ${CMAKE_COMMAND} -E env --unset=COVFILE
    ${CMAKE_CTEST_COMMAND} -C Debug -M Experimental -T Coverage
  )

function(run_MemCheckSan case opts)
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/MemCheckSan${case}-build)
  set(RunCMake_TEST_OPTIONS
    "-DMEMORYCHECK_TYPE=${case}Sanitizer"
    "-DMEMORYCHECK_SANITIZER_OPTIONS=${opts}"
    )
  run_cmake(MemCheckSan)
  unset(RunCMake_TEST_OPTIONS)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake-stdout-file "../ctest_memcheck/Dummy${case}Sanitizer-stdout.txt")
  run_cmake_command(MemCheckSan${case}-ctest
    ${CMAKE_CTEST_COMMAND} -C Debug -M Experimental -T MemCheck -V
    )
endfunction()
run_MemCheckSan(Address "simulate_sanitizer=1:report_bugs=1:history_size=5:exitcode=55")
run_MemCheckSan(Leak "simulate_sanitizer=1:report_bugs=1:history_size=5:exitcode=55")
run_MemCheckSan(Memory "simulate_sanitizer=1:report_bugs=1:history_size=5:exitcode=55")
run_MemCheckSan(Thread "report_bugs=1:history_size=5:exitcode=55")
run_MemCheckSan(UndefinedBehavior "simulate_sanitizer=1")

run_cmake_command(test-dir-invalid-arg ${CMAKE_CTEST_COMMAND} --test-dir)
run_cmake_command(test-dir-non-existing-dir ${CMAKE_CTEST_COMMAND} --test-dir non-existing-dir)

function(run_testDir testName testPreset)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/testDir)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}/sub")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/sub/CTestTestfile.cmake" "
  add_test(Test1 \"${CMAKE_COMMAND}\" -E true)
  add_test(Test2 \"${CMAKE_COMMAND}\" -E true)
  ")
  if (testPreset)
    set(presetCommandLine --preset=default)
    configure_file(
      ${RunCMake_SOURCE_DIR}/testDir-presets.json.in
      ${RunCMake_TEST_BINARY_DIR}/CMakePresets.json)
  endif()
  run_cmake_command(${testName} ${CMAKE_CTEST_COMMAND} --test-dir "${RunCMake_TEST_BINARY_DIR}/sub" ${presetCommandLine})
endfunction()
run_testDir(testDir 0)
run_testDir(testDir-preset 1)

# Test --output-junit
function(run_output_junit)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/output-junit)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
add_test(test1 \"${CMAKE_COMMAND}\" -E false)
add_test(test2 \"${CMAKE_COMMAND}\" -E echo \"hello world\")
add_test(test3 \"${CMAKE_COMMAND}\" -E true)
set_tests_properties(test3 PROPERTIES  DISABLED \"ON\")
add_test(test4 \"${CMAKE_CURRENT_SOURCE_DIR}/does_not_exist\")
add_test(test5 \"${CMAKE_COMMAND}\" -E echo \"please skip\")
set_tests_properties(test5 PROPERTIES  SKIP_REGULAR_EXPRESSION \"please skip\")
")
  run_cmake_command(output-junit ${CMAKE_CTEST_COMMAND} --output-junit "${RunCMake_TEST_BINARY_DIR}/junit.xml")
endfunction()
run_output_junit()

run_cmake_command(invalid-ctest-argument ${CMAKE_CTEST_COMMAND} --not-a-valid-ctest-argument)

if(WIN32)
  block()
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/TimeoutSignalWindows)
    set(RunCMake_TEST_NO_CLEAN 1)
    file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
    file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
    file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
add_test(test1 \"${CMAKE_COMMAND}\" -E true)
set_tests_properties(test1 PROPERTIES TIMEOUT_SIGNAL_NAME SIGUSR1)
set_tests_properties(test1 PROPERTIES TIMEOUT_SIGNAL_GRACE_PERIOD 1)
")
    run_cmake_command(TimeoutSignalWindows ${CMAKE_CTEST_COMMAND})
  endblock()
else()
  block()
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/TimeoutSignalBad)
    set(RunCMake_TEST_NO_CLEAN 1)
    file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
    file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
    file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
add_test(test1 \"${CMAKE_COMMAND}\" -E true)
set_tests_properties(test1 PROPERTIES TIMEOUT_SIGNAL_NAME NOTASIG)
set_tests_properties(test1 PROPERTIES TIMEOUT_SIGNAL_GRACE_PERIOD 0)
set_tests_properties(test1 PROPERTIES TIMEOUT_SIGNAL_GRACE_PERIOD 1000)
")
    run_cmake_command(TimeoutSignalBad ${CMAKE_CTEST_COMMAND})
  endblock()
endif()

block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/ScheduleRandomSeed)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
foreach(i RANGE 1 5)
  add_test(test\${i} \"${CMAKE_COMMAND}\" -E true)
endforeach()
")
  run_cmake_command(ScheduleRandomSeed1 ${CMAKE_CTEST_COMMAND} --schedule-random --schedule-random-seed 42)
  run_cmake_command(ScheduleRandomSeed2 ${CMAKE_CTEST_COMMAND} --schedule-random --schedule-random-seed 42)
endblock()
