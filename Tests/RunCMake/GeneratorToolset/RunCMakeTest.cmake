include(RunCMake)

run_cmake(NoToolset)

set(RunCMake_TEST_OPTIONS -T "Bad Toolset")
run_cmake(BadToolset)
unset(RunCMake_TEST_OPTIONS)

set(RunCMake_TEST_OPTIONS -T "Toolset 1" "-TToolset 2")
run_cmake(TwoToolsets)
unset(RunCMake_TEST_OPTIONS)
