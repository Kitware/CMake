include(RunCMake)

run_cmake(MissingArgument)
run_cmake(EndMissing)
run_cmake(EndMismatch)
run_cmake(EndAlone)
run_cmake(EndAloneArgs)

run_cmake(CMP0130-OLD)
run_cmake(CMP0130-WARN)
run_cmake(CMP0130-NEW)
