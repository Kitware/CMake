
include(RunCMake)

run_cmake(no-arguments)
run_cmake(bad-arguments)
run_cmake(no-source)
run_cmake(bad-source)
run_cmake(no-property)
run_cmake(bad-property)
run_cmake(bad-option)
run_cmake(no-DIRECTORY)
run_cmake(bad-DIRECTORY)
run_cmake(no-TARGET_DIRECTORY)
run_cmake(bad-TARGET_DIRECTORY)

run_cmake(Local)
run_cmake(Subdir)
