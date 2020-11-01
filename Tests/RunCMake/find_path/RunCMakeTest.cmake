include(RunCMake)

run_cmake(EmptyOldStyle)
run_cmake(FromPATHEnv)
run_cmake(PrefixInPATH)
run_cmake(Required)

if(APPLE)
  run_cmake(FrameworksWithSubdirs)
endif()
