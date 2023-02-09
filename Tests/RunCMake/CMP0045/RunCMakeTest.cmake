include(RunCMake)
set(RunCMake_IGNORE_POLICY_VERSION_DEPRECATION ON)

run_cmake(CMP0045-OLD)
run_cmake(CMP0045-NEW)
run_cmake(CMP0045-WARN)
