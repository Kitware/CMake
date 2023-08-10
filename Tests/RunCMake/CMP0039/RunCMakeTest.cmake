include(RunCMake)
set(RunCMake_IGNORE_POLICY_VERSION_DEPRECATION ON)

run_cmake(CMP0039-WARN)
run_cmake(CMP0039-NEW)
run_cmake(CMP0039-OLD)
