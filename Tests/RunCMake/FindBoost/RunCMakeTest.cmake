include(RunCMake)
unset(ENV{Boost_ROOT})

run_cmake(CMakePackage)
run_cmake(NoCXX)
