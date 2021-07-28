include(RunCMake)

run_cmake(shared)
run_cmake(static)
run_cmake(target_link_libraries)
run_cmake(target_link_libraries-cycle1)
run_cmake(target_link_libraries-cycle2)
