include(RunCMake)
set(RunCMake_IGNORE_POLICY_VERSION_DEPRECATION ON)

run_cmake(CMP0055-OLD-Out-of-Scope)
run_cmake(CMP0055-NEW-Out-of-Scope)
run_cmake(CMP0055-WARN-Out-of-Scope)

run_cmake(CMP0055-OLD-Reject-Arguments)
run_cmake(CMP0055-NEW-Reject-Arguments)
run_cmake(CMP0055-WARN-Reject-Arguments)
