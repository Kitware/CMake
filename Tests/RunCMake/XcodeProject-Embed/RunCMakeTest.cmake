include(RunCMake)

# Build a framework that the other tests will use and treat as external.
# Always build in the Debug configuration so that the path to the framework
# is predictable.
function(ExternalFramework)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/ExternalFramework-build)
  set(externalFramework ${RunCMake_TEST_BINARY_DIR}/Debug/sharedFrameworkExt.framework PARENT_SCOPE)

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(ExternalFramework)
  run_cmake_command(ExternalFramework-build
    ${CMAKE_COMMAND} --build ${RunCMake_TEST_BINARY_DIR}
                     --config Debug
                     --target sharedFrameworkExt
  )
endfunction()
ExternalFramework()

set(RunCMake_TEST_OPTIONS -DEXTERNAL_FWK=${externalFramework})

run_cmake(EmbedFrameworksFlagsOff)

function(TestFlagsOn testName)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${testName}-build)

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(${testName})
  run_cmake_command(${testName}-build
    ${CMAKE_COMMAND} --build ${RunCMake_TEST_BINARY_DIR}
                     --config Debug
                     --target app
  )
endfunction()

TestFlagsOn(EmbedFrameworksFlagsOnNoSubdir)
TestFlagsOn(EmbedFrameworksFlagsOnWithSubdir)
