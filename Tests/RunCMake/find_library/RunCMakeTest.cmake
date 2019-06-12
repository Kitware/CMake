include(RunCMake)

run_cmake(Created)
run_cmake(FromPrefixPath)
run_cmake(FromPATHEnv)
if(CMAKE_HOST_UNIX)
  run_cmake(LibArchLink)
endif()
run_cmake(PrefixInPATH)
