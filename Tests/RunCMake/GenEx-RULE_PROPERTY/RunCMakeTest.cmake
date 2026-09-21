
include(RunCMake)

function(run_configure_and_build name)
  run_cmake(${name})
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${name}-build")
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${name}-build ${CMAKE_COMMAND} --build . --config Debug)
endfunction()

run_cmake(no-arguments)
run_cmake(no-rule)
run_cmake(no-property)

run_configure_and_build(RULE_PROPERTY)
