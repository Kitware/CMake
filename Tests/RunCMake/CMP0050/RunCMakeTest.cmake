include(RunCMake)
set(RunCMake_IGNORE_POLICY_VERSION_DEPRECATION ON)

run_cmake(CMP0050-OLD)
run_cmake(CMP0050-NEW)
run_cmake(CMP0050-WARN)
