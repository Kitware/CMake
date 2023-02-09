include(RunCMake)
set(RunCMake_IGNORE_POLICY_VERSION_DEPRECATION ON)

run_cmake(CMP0051-OLD)
run_cmake(CMP0051-NEW)
run_cmake(CMP0051-WARN)
