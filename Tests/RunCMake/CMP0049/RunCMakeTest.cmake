include(RunCMake)
set(RunCMake_IGNORE_POLICY_VERSION_DEPRECATION ON)

run_cmake(CMP0049-OLD)
run_cmake(CMP0049-NEW)
run_cmake(CMP0049-WARN)
