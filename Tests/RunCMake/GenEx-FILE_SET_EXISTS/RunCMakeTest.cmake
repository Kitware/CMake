
include(RunCMake)

run_cmake(no-arguments)
run_cmake(bad-arguments)
run_cmake(no-fileset)
run_cmake(bad-option)
run_cmake(no-TARGET)
run_cmake(bad-TARGET)

run_cmake(FILE_SET_EXISTS)
