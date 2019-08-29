include(RunCMake)

run_cmake(EmptyOldStyle)
run_cmake(FromPATHEnv)
run_cmake(PrefixInPATH)

if(APPLE)
  run_cmake(FrameworksWithSubdirs)
endif()
