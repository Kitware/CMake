include(RunCMake)

function(run_SWIG group language)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${group}${language}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake(${group}${language})
  run_cmake_command(${group}${language}-test ${CMAKE_COMMAND} --build ${RunCMake_TEST_BINARY_DIR} --target Run${language})
endfunction()

run_SWIG(Legacy Python)
run_SWIG(Legacy Perl)
