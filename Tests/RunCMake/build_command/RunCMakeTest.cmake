include(RunCMake)
unset(ENV{CMAKE_CONFIG_TYPE})

run_cmake(ErrorsOFF)
run_cmake(ErrorsON)

set(RunCMake_TEST_OPTIONS -DNoProject=1)
run_cmake(BeforeProject)
unset(RunCMake_TEST_OPTIONS)

run_cmake(CMP0061-NEW)

run_cmake(ParallelLevel)
