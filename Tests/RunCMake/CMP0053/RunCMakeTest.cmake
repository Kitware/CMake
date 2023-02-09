include(RunCMake)
set(RunCMake_IGNORE_POLICY_VERSION_DEPRECATION ON)

run_cmake(CMP0053-OLD)
run_cmake(CMP0053-NEW)
run_cmake(CMP0053-WARN)
