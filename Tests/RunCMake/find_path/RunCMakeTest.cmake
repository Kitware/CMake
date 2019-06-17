include(RunCMake)

run_cmake(PrefixInPATH)

if(APPLE)
  run_cmake(FrameworksWithSubdirs)
endif()
