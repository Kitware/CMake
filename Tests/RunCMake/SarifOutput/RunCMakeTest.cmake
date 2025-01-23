include(RunCMake)
include("${CMAKE_CURRENT_LIST_DIR}/check-sarif.cmake")

# Default case: the SARIF file should not be generated
run_cmake(DefaultSarifOutput)

# Ensure the expected messages are present in the SARIF output
run_cmake_with_options(GenerateSarifResults -DCMAKE_EXPORT_SARIF=ON)

# Activate SARIF output using the `CMAKE_EXPORT_SARIF` variable
run_cmake(ToggleExportSarifVariable)

# If CMake stops with a fatal error, it should still generate a SARIF file if
# requested (and the fatal error should be in the log)
run_cmake_with_options(ProjectFatalError -DCMAKE_EXPORT_SARIF=ON)

# ScriptModeSarifVariable Test: Script mode must ignore the
# `CMAKE_EXPORT_SARIF`variable
run_cmake_script(ScriptModeSarifVariable -DCMAKE_EXPORT_SARIF=ON)

# Check that the command-line option can be used to set the file output path
run_cmake_with_options(SarifFileArgument --sarif-output=test_cmake_run.sarif)

# Test the command-line option in script mode as well
run_cmake_script(SarifFileArgumentScript --sarif-output=test_cmake_run.sarif)
