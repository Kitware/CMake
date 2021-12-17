include(RunCMake)

run_cmake(EmptyOldStyle)
run_cmake(FromPATHEnv)
run_cmake(PrefixInPATH)
run_cmake(Required)
run_cmake(NO_CACHE)

if(APPLE)
  run_cmake(FrameworksWithSubdirs)
endif()

run_cmake_with_options(FromPATHEnvDebugVar --debug-find-var=PATH_IN_ENV_PATH)
