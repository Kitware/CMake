include(RunCMake)

run_cmake(VersionNotGiven)
run_cmake(TooManyVersionsGiven)
run_cmake(InvalidRangeMinVersionNotGiven)
run_cmake(InvalidRangeMaxVersionNotGiven)

set(RunCMake_TEST_OPTIONS "-Dversion=three")
run_cmake(InvalidVersion)
run_cmake(InvalidMaxVersion)
unset(RunCMake_TEST_OPTIONS)
set(RunCMake_TEST_OPTIONS "-Dversion=3.one")
run_cmake(InvalidVersion)
run_cmake(InvalidMaxVersion)
unset(RunCMake_TEST_OPTIONS)

run_cmake(VersionLowerThan2_4)
run_cmake(MinVersionLargerThanMax)
run_cmake(VeryHighVersion)
