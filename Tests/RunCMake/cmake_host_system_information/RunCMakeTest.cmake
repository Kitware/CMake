include(RunCMake)

run_cmake(BadArg1)
run_cmake(BadArg2)
run_cmake(BadArg3)

run_cmake(QueryList)
run_cmake(QueryKeys)

run_cmake(UnitTest)
run_cmake(Exherbo)
run_cmake(Ubuntu)

run_cmake(CentOS6)
run_cmake(Debian6)

run_cmake(UserFallbackScript)

if(RunCMake_GENERATOR MATCHES "^Visual Studio " AND NOT RunCMake_GENERATOR STREQUAL "Visual Studio 9 2008")
  run_cmake(VsMSBuild)
else()
  run_cmake(VsMSBuildMissing)
endif()
