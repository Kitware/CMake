include(RunCMake)
set(RunCMake_IGNORE_POLICY_VERSION_DEPRECATION ON)

run_cmake(CMP0057-OLD)
run_cmake(CMP0057-WARN)
run_cmake(CMP0057-NEW)
