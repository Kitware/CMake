include(RunCMake)

# iOS

set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/iOSFrameworkLayout-build)
set(RunCMake_TEST_NO_CLEAN 1)
set(RunCMake_TEST_OPTIONS "-DCMAKE_TOOLCHAIN_FILE=${RunCMake_SOURCE_DIR}/ios.cmake")

file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

run_cmake(FrameworkLayout)
run_cmake_command(iOSFrameworkLayout-build ${CMAKE_COMMAND} --build .)

unset(RunCMake_TEST_BINARY_DIR)
unset(RunCMake_TEST_NO_CLEAN)
unset(RunCMake_TEST_OPTIONS)

# OSX

set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/OSXFrameworkLayout-build)
set(RunCMake_TEST_NO_CLEAN 1)
set(RunCMake_TEST_OPTIONS "-DCMAKE_TOOLCHAIN_FILE=${RunCMake_SOURCE_DIR}/OSX.cmake")

file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

run_cmake(FrameworkLayout)
run_cmake_command(OSXFrameworkLayout-build ${CMAKE_COMMAND} --build .)

unset(RunCMake_TEST_BINARY_DIR)
unset(RunCMake_TEST_NO_CLEAN)
unset(RunCMake_TEST_OPTIONS)
