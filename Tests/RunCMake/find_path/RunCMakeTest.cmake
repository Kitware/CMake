include(RunCMake)

run_cmake(EmptyOldStyle)
if(WIN32 OR CYGWIN)
  run_cmake(PrefixInPATH)
endif()

if(APPLE)
  run_cmake(FrameworksWithSubdirs)
endif()
