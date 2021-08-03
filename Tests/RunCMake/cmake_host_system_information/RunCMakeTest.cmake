include(RunCMake)

run_cmake(BadArg1)
run_cmake(BadArg2)
run_cmake(BadArg3)

run_cmake(QueryList)
run_cmake(QueryKeys)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")

  run_cmake_with_options(UnitTest)
  run_cmake_with_options(Exherbo)
  run_cmake_with_options(Ubuntu)

  run_cmake_with_options(CentOS6)
  run_cmake_with_options(Debian6)

  run_cmake_with_options(UserFallbackScript)

endif()
