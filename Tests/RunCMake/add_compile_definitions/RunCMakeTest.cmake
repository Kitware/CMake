include(RunCMake)

macro(run_cmake_build test)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${test} ${CMAKE_COMMAND} --build . --config Release)

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
endmacro()

run_cmake(remove_leading_minusD)
run_cmake_build(remove_leading_minusD)
