include(RunCMake)

run_cmake(ModifiedAccess)
run_cmake(NoWatcher)
run_cmake(WatchTwice)
run_cmake(ModifyWatchInCallback)
run_cmake(RaiseInParentScope)
run_cmake(SetToOwnValue)
run_cmake(ReplaceInFirstWatch)
# Two scripts, so that CTest also replaces the objects of the first one.
run_cmake_command(CTestScript ${CMAKE_CTEST_COMMAND}
  -S ${RunCMake_SOURCE_DIR}/CTestScript.cmake
  -S ${RunCMake_SOURCE_DIR}/CTestScript.cmake)
