include(RunCMake)
set(RunCMake_IGNORE_POLICY_VERSION_DEPRECATION ON)

run_cmake(CMP0042-OLD)
run_cmake(CMP0042-NEW)
run_cmake(CMP0042-WARN)
