include(RunCMake)

# Test normal usage
run_cmake(NormalExport)

# Test usage via generator expression
run_cmake(GenExExportCMake)
run_cmake(GenExExportCps)
