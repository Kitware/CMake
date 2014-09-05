include(RunCMake)

run_cmake(NoPlatform)

set(RunCMake_TEST_OPTIONS "-DCMAKE_GENERATOR_PLATFORM=Bad Platform")
run_cmake(BadPlatform)
unset(RunCMake_TEST_OPTIONS)
