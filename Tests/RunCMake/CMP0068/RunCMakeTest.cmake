include(RunCMake)
set(RunCMake_IGNORE_POLICY_VERSION_DEPRECATION ON)

run_cmake(CMP0068-OLD)
run_cmake(CMP0068-NEW)
run_cmake(CMP0068-WARN)
