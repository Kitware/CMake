include(RunCMake)

function(run_compile_warn test)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake(${test})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${test}-Build ${CMAKE_COMMAND} --build . ${verbose_args})
endfunction()

run_compile_warn(WerrorOn)
run_compile_warn(WerrorOff)
