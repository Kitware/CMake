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

block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/rerun)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
add_test(works \"${CMAKE_COMMAND}\" -E true)
add_test(fails \"${CMAKE_COMMAND}\" -E false)
")
  run_cmake_command(rerun-init ${CMAKE_CTEST_COMMAND})
  run_cmake_command(rerun-failed ${CMAKE_CTEST_COMMAND} --rerun-failed)
endblock()

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

run_cmake_command(usage ${CMAKE_CTEST_COMMAND})

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

block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CustomPrePost)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestCustom.cmake" "
set(CTEST_CUSTOM_PRE_TEST \"\\\"${CMAKE_COMMAND}\\\" -E echo \\\"Custom Pre-Test\\\"\")
set(CTEST_CUSTOM_POST_TEST \"\\\"${CMAKE_COMMAND}\\\" -E echo \\\"Custom Post-Test\\\"\")
")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
  add_test(Echo \"${CMAKE_COMMAND}\" -E echo)
")
  run_cmake_command(CustomPrePost ${CMAKE_CTEST_COMMAND})
  run_cmake_command(CustomPrePost-show-only ${CMAKE_CTEST_COMMAND} --show-only=human)
endblock()

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
  ${CMAKE_COMMAND} -E env COVNOSAVE=1 --unset=COVAPPDATADIR --unset=COVFILE
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

# Test --source-dir and --build-dir
run_cmake_command(source-dir-invalid-arg ${CMAKE_CTEST_COMMAND} --source-dir)
run_cmake_command(build-dir-invalid-arg ${CMAKE_CTEST_COMMAND} --build-dir)

# Build the -D arguments needed to pass the generator to ctest -T Configure.
macro(ctest_source_dir_generator_args var)
  set(${var} -D "CTEST_CMAKE_GENERATOR=${RunCMake_GENERATOR}")
  if(RunCMake_GENERATOR_PLATFORM)
    list(APPEND ${var}
      -D "CTEST_CMAKE_GENERATOR_PLATFORM=${RunCMake_GENERATOR_PLATFORM}")
  endif()
endmacro()

# Verify that ctest -T Configure works with an empty binary directory
# when --source-dir is provided.
function(run_configure_empty_bindir)
  set(src "${RunCMake_BINARY_DIR}/configure-empty-bindir-src")
  set(bin "${RunCMake_BINARY_DIR}/configure-empty-bindir-bin")
  file(REMOVE_RECURSE "${src}" "${bin}")
  file(MAKE_DIRECTORY "${src}")
  file(WRITE "${src}/CMakeLists.txt"
    "cmake_minimum_required(VERSION 3.10)\nproject(Minimal LANGUAGES NONE)\n")
  # Note that run_cmake_command always pre-creates RunCMake_TEST_BINARY_DIR
  # before invoking the command, so this test exercises configure from an already-
  # created but otherwise empty binary directory.  The MakeDirectory call in CTest
  # itself is what would handle the truly-absent case in production use.
  set(RunCMake_TEST_BINARY_DIR "${bin}")
  set(RunCMake_TEST_NO_CLEAN 1)
  ctest_source_dir_generator_args(generator_args)
  run_cmake_command(configure-empty-bindir
    ${CMAKE_CTEST_COMMAND}
    --source-dir "${src}"
    --build-dir  "${bin}"
    ${generator_args}
    -T Configure)
endfunction()
run_configure_empty_bindir()

# Verify expected error condition when --source-dir does not contain
# a CMakeLists.txt file.
function(run_configure_no_cmakelists)
  set(src "${RunCMake_BINARY_DIR}/configure-no-cmakelists-src")
  set(bin "${RunCMake_BINARY_DIR}/configure-no-cmakelists-bin")
  file(REMOVE_RECURSE "${src}" "${bin}")
  # Source dir exists but contains no CMakeLists.txt
  file(MAKE_DIRECTORY "${src}")
  set(RunCMake_TEST_BINARY_DIR "${bin}")
  set(RunCMake_TEST_NO_CLEAN 1)
  ctest_source_dir_generator_args(generator_args)
  run_cmake_command(configure-no-cmakelists
    ${CMAKE_CTEST_COMMAND}
    --source-dir "${src}"
    --build-dir  "${bin}"
    ${generator_args}
    -T Configure)
endfunction()
run_configure_no_cmakelists()

# Helper for tests that invoke ctest -M/-T Configure with -D CTEST_PRESET.
# Options:
#   SOURCE_DIR  -- pass --source-dir to ctest
#   BUILD_DIR   -- create a separate <CASE_NAME>-build dir with
#                  DartConfiguration.tcl and pass --build-dir to ctest
#   DART_VARS   -- seed DartConfiguration.tcl with BuildName/Site originals
#                  and pass -D CTEST_BUILD_NAME/CTEST_SITE overrides to ctest
function(run_ctest_configure_cli_preset CASE_NAME)
  cmake_parse_arguments(PARSE_ARGV 1 ARG "SOURCE_DIR;BUILD_DIR;BAD_PRESETS;DART_VARS" "PRESET_NAME" "")
  set(src "${RunCMake_BINARY_DIR}/${CASE_NAME}")
  set(bin "${src}")
  if(ARG_BUILD_DIR)
    set(bin "${RunCMake_BINARY_DIR}/${CASE_NAME}-build")
  endif()
  set(preset_name "my-preset")
  if(ARG_PRESET_NAME)
    set(preset_name "${ARG_PRESET_NAME}")
  endif()
  configure_file("${RunCMake_SOURCE_DIR}/CMakeLists.txt.in"
                 "${src}/CMakeLists.txt" @ONLY)
  if(ARG_BAD_PRESETS)
    configure_file("${RunCMake_SOURCE_DIR}/BadCMakePresets.json.in"
                   "${src}/CMakePresets.json" @ONLY)
  else()
    configure_file("${RunCMake_SOURCE_DIR}/CMakePresets.json.in"
                   "${src}/CMakePresets.json" @ONLY)
  endif()
  file(REMOVE_RECURSE "${src}/build")
  if(ARG_BUILD_DIR)
    file(REMOVE_RECURSE "${bin}")
    file(MAKE_DIRECTORY "${bin}")
    set(dart_extra "")
    if(ARG_DART_VARS)
      set(dart_extra "BuildName: original-build-name\nSite: original-site\n")
    endif()
    file(WRITE "${bin}/DartConfiguration.tcl"
      "BuildDirectory: ${bin}\n"
      "SourceDirectory: ${src}\n"
      "${dart_extra}"
      "ConfigureCommand: \"${CMAKE_COMMAND}\" -S\"${src}\" -B\"${bin}\"\n")
  endif()
  set(extra_args "")
  if(ARG_SOURCE_DIR)
    list(APPEND extra_args --source-dir "${src}")
  endif()
  if(ARG_BUILD_DIR)
    list(APPEND extra_args --build-dir "${bin}")
  endif()
  if(ARG_DART_VARS)
    list(APPEND extra_args
      -D "CTEST_BUILD_NAME=cli-build-name"
      -D "CTEST_SITE=cli-site")
  endif()
  set(RunCMake_TEST_SOURCE_DIR "${src}")
  set(RunCMake_TEST_BINARY_DIR "${bin}")
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${CASE_NAME}
    ${CMAKE_CTEST_COMMAND}
    ${extra_args}
    -M Experimental
    -D "CTEST_PRESET=${preset_name}"
    -T Configure
    -V)
endfunction()

# CTEST_PRESET via -D reaches ctest_configure() when run with -M/-T.
run_ctest_configure_cli_preset(ConfigurePresetCLIVar BUILD_DIR)

# --source-dir + -D CTEST_PRESET picks up the preset's binaryDir when no
# DartConfiguration.tcl / explicit --build-dir is provided.
run_ctest_configure_cli_preset(ConfigurePresetCLIVarSourceDir SOURCE_DIR)

# An explicit --build-dir takes precedence over the preset's binaryDir.
run_ctest_configure_cli_preset(ConfigurePresetCLIVarBuildDirOverride SOURCE_DIR BUILD_DIR)

# A malformed presets file is an error.
run_ctest_configure_cli_preset(ConfigurePresetCLIVarBadPresets SOURCE_DIR BAD_PRESETS)

# Referencing a preset that does not exist is an error.
run_ctest_configure_cli_preset(ConfigurePresetCLIVarUnknownPreset SOURCE_DIR
  PRESET_NAME nonexistent-preset)

# -D CTEST_BUILD_NAME and -D CTEST_SITE are recorded in DartConfiguration.tcl,
# overriding pre-existing values (pre-existing binary dir).
run_ctest_configure_cli_preset(ConfigureCLIVarDartPersist BUILD_DIR DART_VARS)

# -D CTEST_BUILD_NAME and -D CTEST_SITE are recorded in DartConfiguration.tcl
# when cmake creates it fresh (empty binary dir via --source-dir).
run_ctest_configure_cli_preset(ConfigureCLIVarDartPersistSourceDir SOURCE_DIR DART_VARS)

# The following end-to-end test demonstrates that:
# 1) ctest -T Configure writes user-specified Site and BuildName in
#    preset.binaryDir/DartConfiguration.tcl.
# 2) ctest -T Update correctly loads this data.
block()
  set(src "${RunCMake_BINARY_DIR}/ReuseBuildNameFromPresetBinaryDir")
  set(bin "${src}/build")
  configure_file("${RunCMake_SOURCE_DIR}/CMakeLists.txt.in"
                 "${src}/CMakeLists.txt" @ONLY)
  configure_file("${RunCMake_SOURCE_DIR}/CMakePresets.json.in"
                 "${src}/CMakePresets.json" @ONLY)
  file(REMOVE_RECURSE "${bin}")
  ctest_source_dir_generator_args(generator_args)
  set(RunCMake_TEST_SOURCE_DIR "${src}")
  set(RunCMake_TEST_BINARY_DIR "${bin}")
  set(RunCMake_TEST_NO_CLEAN 1)
  # Step 1: ctest -T Configure sets Site/BuildName in DartConfiguration.tcl.
  run_cmake_command(ReuseBuildNameFromPresetBinaryDir-configure
    ${CMAKE_CTEST_COMMAND}
    --source-dir "${src}"
    ${generator_args}
    -M Experimental
    -T Configure
    -D "CTEST_PRESET=my-preset"
    -D "CTEST_SITE=my-site"
    -D "CTEST_BUILD_NAME=my-build-name")
  # Step 2: ctest -T Update reuses Site/BuildName from above.
  run_cmake_command(ReuseBuildNameFromPresetBinaryDir-update
    ${CMAKE_CTEST_COMMAND}
    --source-dir "${src}"
    -M Experimental
    -T Update
    -D "CTEST_PRESET=my-preset"
    -D "CTEST_UPDATE_COMMAND=${CMAKE_COMMAND}"
    -D "CTEST_UPDATE_VERSION_ONLY=1"
    -V)
endblock()

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

block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/TimeoutDefault)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/DartConfiguration.tcl" "
BuildDirectory: ${RunCMake_TEST_BINARY_DIR}
")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
add_test(test1 \"${CMAKE_COMMAND}\" -E true)
")
  run_cmake_command(TimeoutDefault ${CMAKE_CTEST_COMMAND} -V)
  run_cmake_command(TimeoutDefault-T-Test ${CMAKE_CTEST_COMMAND} -V -T Test)
endblock()

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

function(run_PassthroughTest case)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/Passthrough-${case})
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
add_test(echo_test \"${CMAKE_COMMAND}\" -E echo \"base_output\")
add_test(echo_test2 \"${CMAKE_COMMAND}\" -E echo \"second_test\")
")
  run_cmake_command(Passthrough-${case} ${CMAKE_CTEST_COMMAND} ${ARGN})
endfunction()

# Basic passthrough
run_PassthroughTest(basic -V -- --extra-flag)
# Multiple passthrough args
run_PassthroughTest(multiple -V -- --flag1 --flag2 value)
# Passthrough with -R filter (only echo_test2 should run)
run_PassthroughTest(with-filter -V -R echo_test2 -- --extra)
# Empty passthrough (bare --)
run_PassthroughTest(empty -V --)
# Passthrough args that look like ctest flags
run_PassthroughTest(ctest-flags -V -- -R --verbose)
# -- rejected with --build-and-test
run_cmake_command(Passthrough-build-and-test-error ${CMAKE_CTEST_COMMAND}
  --build-and-test
    ${RunCMake_BINARY_DIR}/Passthrough-build-and-test-error/does-not-exist
    ${RunCMake_BINARY_DIR}/Passthrough-build-and-test-error/does-not-exist
  --build-generator "None" -- --extra-flag)
# bare -- rejected with --build-and-test
run_cmake_command(Passthrough-build-and-test-empty-error ${CMAKE_CTEST_COMMAND}
  --build-and-test
    ${RunCMake_BINARY_DIR}/Passthrough-build-and-test-empty-error/does-not-exist
    ${RunCMake_BINARY_DIR}/Passthrough-build-and-test-empty-error/does-not-exist
  --build-generator "None" --)

block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CoverageTool)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/DartConfiguration.tcl" "
BuildDirectory: ${RunCMake_TEST_BINARY_DIR}
CTestTestCoverageTool: LLVM-COV
")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
add_test(test1 \"${CMAKE_COMMAND}\" -E true)
")
  run_cmake_command(CoverageTool ${CMAKE_CTEST_COMMAND} -V)
  run_cmake_command(CoverageTool-T-Test ${CMAKE_CTEST_COMMAND} -V -T Test)
endblock()

# Verify that CTEST_* variables defined via the command-line are visible
# from the CTest script.
run_cmake_command(DashD-ScriptVars
  ${CMAKE_CTEST_COMMAND}
  "-S" "${RunCMake_SOURCE_DIR}/DashD-ScriptVars.cmake"
  "-D" "CTEST_BUILD_NAME=cli-build-name"
  "-D" "CTEST_SITE=cli-site"
  "-D" "CTEST_BUILD_FLAGS=-O2 -Wall"
  "-D" "CTEST_BUILD_TARGET=my-target"
  "-D" "CTEST_CMAKE_GENERATOR=Ninja"
  "-D" "CTEST_EXTRA_COVERAGE_GLOB=**/*.gcov"
  "-D" "CTEST_TIME_LIMIT=3600"
  )

# Verify that CTEST_* variables defined via the command-line override
# settings from the dashboard configuration file.
block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/DashD-DashboardVar-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/DartConfiguration.tcl"
    "BuildName: original-build-name\nSite: original-site\nSourceDirectory: ${RunCMake_TEST_BINARY_DIR}\n")
  run_cmake_command(DashD-DashboardVar
    ${CMAKE_CTEST_COMMAND} -M Experimental -T Start
    -D CTEST_BUILD_NAME=cli-build-name
    )
endblock()

# Test CTEST_SUBMIT_PARTS: a valid part name reaches the network submission step.
block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/SubmitParts-valid-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/DartConfiguration.tcl"
    "SourceDirectory: ${RunCMake_TEST_BINARY_DIR}\n"
    "BuildDirectory: ${RunCMake_TEST_BINARY_DIR}\n"
    "DropMethod: https\n"
    "DropSite: badhostname.invalid\n"
    "DropLocation: /submit.php?project=Test\n"
    "CTestSubmitRetryCount: 0\n"
  )
  run_cmake_command(SubmitParts-valid-ctest
    ${CMAKE_CTEST_COMMAND} -M Experimental -T Start -T Submit -VV
    -D CTEST_SUBMIT_PARTS=Done
  )
endblock()

# Test CTEST_SUBMIT_PARTS: an invalid part name produces a validation error.
block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/SubmitParts-badpart-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/DartConfiguration.tcl"
    "SourceDirectory: ${RunCMake_TEST_BINARY_DIR}\n"
    "BuildDirectory: ${RunCMake_TEST_BINARY_DIR}\n"
    "DropMethod: https\n"
    "DropSite: badhostname.invalid\n"
    "DropLocation: /submit.php?project=Test\n"
    "CTestSubmitRetryCount: 0\n"
  )
  run_cmake_command(SubmitParts-badpart-ctest
    ${CMAKE_CTEST_COMMAND} -M Experimental -T Start -T Submit
    -D CTEST_SUBMIT_PARTS=BadPart
  )
endblock()
