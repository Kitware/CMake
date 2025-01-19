include(RunCMake)

run_cmake(CMP0022-NOWARN-shared)
run_cmake(CMP0022-NOWARN-static)
run_cmake(CMP0022-NOWARN-static-NEW)
run_cmake(CMP0022-NOWARN-static-link_libraries)
run_cmake(CMP0022-export)
run_cmake(CMP0022-export-exe)
run_cmake(CMP0022-install-export)
