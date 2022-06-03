
include(RunCMake)

run_cmake(WrongArguments)

function(check_path_execution name)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${name}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_VARIANT_DESCRIPTION " - ${name}")
  run_cmake_with_options(generate -DPATH_TEST=${name})
  run_cmake_command(check "${CMAKE_COMMAND}" "-DRunCMake_SOURCE_DIR=${RunCMake_SOURCE_DIR}" -P "${RunCMake_TEST_BINARY_DIR}/${name}.cmake")
endfunction()

check_path_execution (PATH_EQUAL)
