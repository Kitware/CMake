include(RunCMake)

run_cmake(Created)
run_cmake(FromPrefixPath)
run_cmake(FromPATHEnv)
if(UNIX AND NOT CYGWIN)
  run_cmake(LibArchLink)
  run_cmake(LibSymLink)
endif()
run_cmake(PrefixInPATH)
run_cmake(Required)
run_cmake(NO_CACHE)

run_cmake_script(FromScriptMode "-DTEMP_DIR=${RunCMake_BINARY_DIR}/FromScriptMode-temp")

run_cmake_with_options(FromPATHEnvDebugVar --debug-find-var=CREATED_LIBRARY)
