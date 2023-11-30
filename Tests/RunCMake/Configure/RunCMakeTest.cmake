include(RunCMake)

run_cmake(ContinueAfterError)
run_cmake(CopyFileABI)
run_cmake(CustomTargetAfterError)

# Use a single build tree for a few tests without cleaning.
set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/RerunCMake-build)
set(RunCMake_TEST_NO_CLEAN 1)
file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
set(input  "${RunCMake_TEST_BINARY_DIR}/CustomCMakeInput.txt")
set(stamp  "${RunCMake_TEST_BINARY_DIR}/CustomCMakeStamp.txt")
set(depend "${RunCMake_TEST_BINARY_DIR}/CustomCMakeDepend.txt")
set(output "${RunCMake_TEST_BINARY_DIR}/CustomCMakeOutput.txt")
set(error  "${RunCMake_TEST_BINARY_DIR}/CustomCMakeError.txt")
file(WRITE "${input}" "1")
file(WRITE "${depend}" "1")
run_cmake(RerunCMake)
execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 1) # handle 1s resolution
file(WRITE "${input}" "2")
run_cmake_command(RerunCMake-build1 ${CMAKE_COMMAND} --build .)
file(WRITE "${depend}" "2")
run_cmake_command(RerunCMake-build2 ${CMAKE_COMMAND} --build .)
execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 1) # handle 1s resolution
file(WRITE "${depend}" "3")
file(WRITE "${error}" "3")
set(RunCMake_TEST_OUTPUT_MERGE 1)
run_cmake_command(RerunCMake-build3 ${CMAKE_COMMAND} --build .)
if(MSVC_IDE)
  # Make sure that for Visual Studio the error occurs from within the build
  # system.
  file(REMOVE "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/generate.stamp.list")
  file(WRITE "${error}" "4")
  # With Visual Studio the error must be on stdout, otherwise the error was not
  # emitted by ZERO_CHECK.
  set(RunCMake_TEST_OUTPUT_MERGE 0)
  run_cmake_command(RerunCMake-build4 ${CMAKE_COMMAND} --build .)
endif()
unset(RunCMake_TEST_OUTPUT_MERGE)
unset(RunCMake_TEST_BINARY_DIR)
unset(RunCMake_TEST_NO_CLEAN)

# Use a single build tree for a few tests without cleaning.
set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/RemoveCache-build)
set(RunCMake_TEST_NO_CLEAN 1)
file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
run_cmake(RemoveCache)
file(REMOVE "${RunCMake_TEST_BINARY_DIR}/CMakeCache.txt")
run_cmake(RemoveCache)

if(NOT RunCMake_GENERATOR MATCHES "^Ninja Multi-Config$")
  run_cmake(NoCMAKE_CROSS_CONFIGS)
  run_cmake(NoCMAKE_DEFAULT_BUILD_TYPE)
  run_cmake(NoCMAKE_DEFAULT_CONFIGS)
endif()

if(NOT CMAKE_HOST_WIN32)
  block()
    # Test a non-writable build directory.
    # Exclude when running as root because directories are always writable.
    get_unix_uid(uid)
    if(NOT uid STREQUAL "0")
      set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/ReadOnly-build)
      file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
      file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
      file(CHMOD "${RunCMake_TEST_BINARY_DIR}" PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
      set(RunCMake_TEST_NO_CLEAN 1)
      run_cmake(ReadOnly)
    endif()
  endblock()
endif()
