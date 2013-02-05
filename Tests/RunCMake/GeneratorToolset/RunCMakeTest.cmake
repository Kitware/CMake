include(RunCMake)

set(RunCMake_GENERATOR_TOOLSET "")
run_cmake(NoToolset)

if("${RunCMake_GENERATOR}" MATCHES "Visual Studio 1[01]|Xcode")
  set(RunCMake_GENERATOR_TOOLSET "Test Toolset")
  run_cmake(TestToolset)
else()
  set(RunCMake_GENERATOR_TOOLSET "Bad Toolset")
  run_cmake(BadToolset)
endif()

set(RunCMake_GENERATOR_TOOLSET "")
set(RunCMake_TEST_OPTIONS -T "Extra Toolset")
run_cmake(TwoToolsets)
unset(RunCMake_TEST_OPTIONS)
