include(RunCMake)

function(run_MacOSVersions)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/MacOSVersions-build)
  run_cmake(MacOSVersions)

  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(MacOSVersions-build ${CMAKE_COMMAND} --build . --config Debug)
endfunction()

run_MacOSVersions()
