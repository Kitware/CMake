include(RunCMake)

run_cmake(NoToolset)

if("${RunCMake_GENERATOR}" MATCHES "Visual Studio 1[01]|Xcode" AND NOT XCODE_BELOW_3)
  set(RunCMake_TEST_OPTIONS -T "Test Toolset")
  run_cmake(TestToolset)
  unset(RunCMake_TEST_OPTIONS)
else()
  set(RunCMake_TEST_OPTIONS -T "Bad Toolset")
  run_cmake(BadToolset)
  unset(RunCMake_TEST_OPTIONS)
endif()

set(RunCMake_TEST_OPTIONS -T "Toolset 1" "-TToolset 2")
run_cmake(TwoToolsets)
unset(RunCMake_TEST_OPTIONS)
