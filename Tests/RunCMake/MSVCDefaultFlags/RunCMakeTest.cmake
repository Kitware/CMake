include(RunCMake)

run_cmake(CMP0092-WARN)
run_cmake(CMP0092-OLD)
run_cmake(CMP0092-NEW)

function(run_CMP0197 pol)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CMP0197-${pol})
  run_cmake(CMP0197-${pol})
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  set(RunCMake-stdout-file "CMP0197-build-stdout.txt")
  run_cmake_command(CMP0197-${pol}-build ${CMAKE_COMMAND} --build . --config Debug --verbose)
endfunction()

run_CMP0197(WARN)
run_CMP0197(OLD)
run_CMP0197(NEW)
