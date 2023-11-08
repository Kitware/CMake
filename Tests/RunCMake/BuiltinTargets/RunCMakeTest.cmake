include(RunCMake)

if(RunCMake_GENERATOR MATCHES "Make|Ninja")
  set(test_target "test")
else()
  set(test_target "RUN_TESTS")
endif()

function(run_BuiltinTarget case target)
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-build)
  run_cmake(${case})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${case}-build ${CMAKE_COMMAND} --build . --config Debug --target ${${target}_target})
endfunction()

run_BuiltinTarget(TestDependsAll-Default test)
run_BuiltinTarget(TestDependsAll-No test)
run_BuiltinTarget(TestDependsAll-Yes test)
