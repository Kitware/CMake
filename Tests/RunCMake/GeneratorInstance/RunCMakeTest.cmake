include(RunCMake)

set(RunCMake_GENERATOR_INSTANCE "")
run_cmake(NoInstance)

set(RunCMake_GENERATOR_INSTANCE "Bad Instance")
run_cmake(BadInstance)

set(RunCMake_TEST_OPTIONS -DCMAKE_TOOLCHAIN_FILE=${RunCMake_SOURCE_DIR}/BadInstance-toolchain.cmake)
run_cmake(BadInstanceToolchain)
unset(RunCMake_TEST_OPTIONS)
