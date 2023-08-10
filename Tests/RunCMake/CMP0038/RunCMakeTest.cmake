include(RunCMake)
set(RunCMake_IGNORE_POLICY_VERSION_DEPRECATION ON)

run_cmake(CMP0038-WARN)
run_cmake(CMP0038-NEW)
run_cmake(CMP0038-OLD)
