include(RunCMake)

run_cmake(CMP0126-OLD)
run_cmake_with_options(CMP0126-OLD_CL -DVAR=3)
run_cmake(CMP0126-NEW)
run_cmake_with_options(CMP0126-NEW_CL -DVAR=3)
run_cmake(CMP0126-WARN)
run_cmake(CMP0126-WARN-default)
