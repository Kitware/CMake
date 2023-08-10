include(RunCMake)

run_cmake(DIRECTORY-invalid)

set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/DIRECTORY-build)
run_cmake(DIRECTORY)
set(RunCMake_TEST_NO_CLEAN 1)
run_cmake_command(DIRECTORY-test ${CMAKE_CTEST_COMMAND} -C Debug)
