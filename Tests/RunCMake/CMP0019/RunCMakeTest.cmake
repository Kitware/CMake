include(RunCMake)
set(RunCMake_IGNORE_POLICY_VERSION_DEPRECATION ON)

run_cmake(CMP0019-WARN)
run_cmake(CMP0019-OLD)
run_cmake(CMP0019-NEW)
