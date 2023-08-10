include(RunCMake)
set(RunCMake_IGNORE_POLICY_VERSION_DEPRECATION ON)

run_cmake(NotClosed)
run_cmake(NotOpened)
run_cmake(parent-dir-generate-time)
run_cmake(dir-in-macro-generate-time)
