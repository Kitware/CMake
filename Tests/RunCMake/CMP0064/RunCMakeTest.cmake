include(RunCMake)
set(RunCMake_IGNORE_POLICY_VERSION_DEPRECATION ON)

run_cmake(CMP0064-OLD)
run_cmake(CMP0064-WARN)
run_cmake(CMP0064-NEW)
