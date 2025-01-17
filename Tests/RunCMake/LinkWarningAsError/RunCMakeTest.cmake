include(RunCMake)

run_cmake(BadValue)

function(run_link_warn test)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_with_options(${test} ${ARGN})
  set(RunCMake_TEST_NO_CLEAN 1)
  if(ARGN MATCHES "--link-no-warning-as-error")
    # Cause the build system to re-run CMake to verify that this option is preserved.
    run_cmake_command(${test}-Touch ${CMAKE_COMMAND} -E touch_nocreate CMakeCache.txt)
  endif()
  run_cmake_command(${test}-Build ${CMAKE_COMMAND} --build . --verbose)
endfunction()

run_link_warn(WarnErrorOn1)
run_link_warn(WarnErrorOn2)
run_link_warn(WarnErrorOn3)
run_link_warn(WarnErrorOn4)
run_link_warn(WarnErrorOff1)
run_link_warn(WarnErrorOff2)
run_link_warn(WarnErrorOnIgnore "--link-no-warning-as-error")
