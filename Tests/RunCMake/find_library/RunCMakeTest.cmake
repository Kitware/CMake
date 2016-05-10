include(RunCMake)

run_cmake(Created)
if(WIN32 OR CYGWIN)
  run_cmake(PrefixInPATH)
endif()
