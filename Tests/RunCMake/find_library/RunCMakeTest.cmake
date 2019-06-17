include(RunCMake)

run_cmake(Created)
if(CMAKE_HOST_UNIX)
  run_cmake(LibArchLink)
endif()
run_cmake(PrefixInPATH)
