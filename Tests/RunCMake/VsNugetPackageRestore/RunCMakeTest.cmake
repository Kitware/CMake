cmake_policy(SET CMP0053 NEW)
include(RunCMake)

set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/VsNugetPackageRestore)
run_cmake(VsNugetPackageRestore)

set(RunCMake_TEST_NO_CLEAN 1)
run_cmake_command(vs-nuget-package-restore-off ${CMAKE_COMMAND} --build ${RunCMake_TEST_BINARY_DIR} --resolve-package-references=off)
run_cmake_command(vs-nuget-package-restore-only ${CMAKE_COMMAND} --build ${RunCMake_TEST_BINARY_DIR} --resolve-package-references=only)
run_cmake_command(vs-nuget-package-restore-on ${CMAKE_COMMAND} --build ${RunCMake_TEST_BINARY_DIR} --resolve-package-references=on)
run_cmake_command(vs-nuget-package-restore-wrong ${CMAKE_COMMAND} --build ${RunCMake_TEST_BINARY_DIR} --resolve-package-references=wrong)
set(RunCMake_TEST_NO_CLEAN 0)
