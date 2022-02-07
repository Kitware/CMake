include(RunCTest)

set(CASE_CTEST_BUILD_ARGS "")
set(RunCMake_USE_LAUNCHERS TRUE)
set(RunCMake_USE_CUSTOM_BUILD_COMMAND FALSE)

function(run_ctest_build CASE_NAME)
  set(CASE_CTEST_BUILD_ARGS "${ARGN}")
  run_ctest(${CASE_NAME})
endfunction()

run_ctest_build(BuildQuiet QUIET)
run_ctest_build(ParallelLevel PARALLEL_LEVEL 1)

function(run_BuildFailure)
  set(CASE_CMAKELISTS_SUFFIX_CODE [[
add_custom_target(BuildFailure ALL COMMAND command-does-not-exist)
]])
  set(CASE_CMAKELISTS_PREFIX_CODE [[
if(NOT CTEST_USE_LAUNCHERS)
  message(FATAL_ERROR "CTEST_USE_LAUNCHERS not set")
endif()
]])
  set(CASE_TEST_PREFIX_CODE [[
cmake_policy(SET CMP0061 NEW)
]])
  set(CASE_TEST_SUFFIX_CODE [[
if (ctest_build_return_value)
  message("ctest_build returned non-zero")
else()
  message("ctest_build returned zero")
endif()
]])
  run_ctest(BuildFailure)

  if (RunCMake_GENERATOR MATCHES "Makefiles")
    set(CASE_TEST_PREFIX_CODE "")
    run_ctest(BuildFailure-CMP0061-OLD)
  endif()
endfunction()
run_BuildFailure()

function(run_BuildChangeId)
  set(CASE_TEST_PREFIX_CODE [[
    set(CTEST_CHANGE_ID "<>1")
  ]])

  run_ctest(BuildChangeId)
endfunction()
run_BuildChangeId()

function(run_SubdirTarget)
  set(CASE_CMAKELISTS_SUFFIX_CODE [=[
file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/subdir/CMakeLists.txt [[
add_custom_target(target_in_subdir COMMAND ${CMAKE_COMMAND} -E touch target_in_subdir.out VERBATIM)
]])
add_subdirectory(subdir)
]=])
  set(CASE_CTEST_BUILD_ARGS TARGET target_in_subdir)
  run_ctest(SubdirTarget)
endfunction()
run_SubdirTarget()

set(RunCMake_USE_CUSTOM_BUILD_COMMAND TRUE)
set(RunCMake_BUILD_COMMAND "${FAKE_BUILD_COMMAND_EXE}")
run_ctest(BuildCommandFailure)
unset(RunCMake_BUILD_COMMAND)

set(RunCMake_USE_LAUNCHERS FALSE)
set(RunCMake_BUILD_COMMAND "${COLOR_WARNING}")
run_ctest(IgnoreColor)
unset(RunCMake_BUILD_COMMAND)
