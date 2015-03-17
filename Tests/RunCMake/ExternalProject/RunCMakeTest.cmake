include(RunCMake)

run_cmake(CMAKE_CACHE_ARGS)
run_cmake(CMAKE_CACHE_DEFAULT_ARGS)
run_cmake(CMAKE_CACHE_mix)
run_cmake(NO_DEPENDS)
run_cmake(Add_StepDependencies)
run_cmake(Add_StepDependencies_no_target)
