include(RunCMake)

run_cmake(DirsPerName)

if(CMAKE_SYSTEM_NAME MATCHES "^(Windows|CYGWIN)$")
  run_cmake(WindowsCom)
  run_cmake(WindowsExe)
endif()
