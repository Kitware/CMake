include(RunCMake)
set(RunCMake_IGNORE_POLICY_VERSION_DEPRECATION ON)

run_cmake(CMP0069-OLD)
run_cmake(CMP0069-NEW-cmake)
run_cmake(CMP0069-NEW-compiler)
run_cmake(CMP0069-WARN)
