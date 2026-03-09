include(RunCMake)

function(run_cmake_case_cmp0212 case)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-build)
  run_cmake(${case})
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(${case}-build ${CMAKE_COMMAND} --build . --target ${ARGN})
endfunction()

run_cmake_case_cmp0212(CMP0212-OLD tgt1)
run_cmake_case_cmp0212(CMP0212-NEW tgt1 bar)
