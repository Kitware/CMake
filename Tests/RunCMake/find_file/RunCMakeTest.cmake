include(RunCMake)

run_cmake(FromPATHEnv)
run_cmake(FromPrefixPath)
run_cmake(PrefixInPATH)
run_cmake(Required)
run_cmake(NO_CACHE)

run_cmake_with_options(FromPATHEnvDebugVar --debug-find-var=PrefixInPATH_File)
