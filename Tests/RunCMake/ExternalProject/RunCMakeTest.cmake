include(RunCMake)

run_cmake(NoOptions)
run_cmake(SourceEmpty)
run_cmake(SourceMissing)
run_cmake(CMAKE_CACHE_ARGS)
run_cmake(CMAKE_CACHE_DEFAULT_ARGS)
run_cmake(CMAKE_CACHE_mix)
run_cmake(NO_DEPENDS)
run_cmake(Add_StepDependencies)
run_cmake(Add_StepDependencies_iface)
run_cmake(Add_StepDependencies_iface_step)
run_cmake(Add_StepDependencies_no_target)
run_cmake(UsesTerminal)

# Run both cmake and build steps. We always do a clean before the
# build to ensure that the download step re-runs each time.
set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/MultiCommand-build)
set(RunCMake_TEST_NO_CLEAN 1)
file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
run_cmake(MultiCommand)
run_cmake_command(MultiCommand-clean ${CMAKE_COMMAND} --build . --target clean)
run_cmake_command(MultiCommand-build ${CMAKE_COMMAND} --build .)
