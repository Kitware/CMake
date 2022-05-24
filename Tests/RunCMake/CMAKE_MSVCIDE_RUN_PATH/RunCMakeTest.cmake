include(RunCMake)

set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CheckEnvironmentVar-build)
run_cmake(CheckEnvironmentVar)
set(RunCMake_TEST_NO_CLEAN 1)
run_cmake_command(CheckEnvironmentVar-build ${CMAKE_COMMAND} --build . --config Debug --target main)
unset(RunCMake_TEST_BINARY_DIR)
unset(RunCMake_TEST_NO_CLEAN)
