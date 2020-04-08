include(RunCMake)

function(run_GoogleTest DISCOVERY_MODE)
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/GoogleTest-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  if(NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
  endif()
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake_with_options(GoogleTest -DCMAKE_GTEST_DISCOVER_TESTS_DISCOVERY_MODE=${DISCOVERY_MODE})

  run_cmake_command(GoogleTest-build
    ${CMAKE_COMMAND}
    --build .
    --config Debug
    --target fake_gtest
  )

  run_cmake_command(GoogleTest-property-timeout-exe
    ${CMAKE_COMMAND}
    --build .
    --config Debug
    --target property_timeout_test
  )

  run_cmake_command(GoogleTest-test1
    ${CMAKE_CTEST_COMMAND}
    -C Debug
    -L TEST1
    --no-label-summary
  )

  run_cmake_command(GoogleTest-test2
    ${CMAKE_CTEST_COMMAND}
    -C Debug
    -L TEST2
    --no-label-summary
  )

  run_cmake_command(GoogleTest-test-missing
    ${CMAKE_CTEST_COMMAND}
    -C Debug
    -R no_tests_defined
    --no-label-summary
  )

  run_cmake_command(GoogleTest-property-timeout1
    ${CMAKE_CTEST_COMMAND}
    -C Debug
    -R property_timeout\\.case_no_discovery
    --no-label-summary
  )

  run_cmake_command(GoogleTest-property-timeout2
    ${CMAKE_CTEST_COMMAND}
    -C Debug
    -R property_timeout\\.case_with_discovery
    --no-label-summary
  )

  run_cmake_command(GoogleTest-build
    ${CMAKE_COMMAND}
    --build .
    --config Debug
    --target skip_test
  )

  run_cmake_command(GoogleTest-skip-test
    ${CMAKE_CTEST_COMMAND}
    -C Debug
    -R skip_test
    --no-label-summary
  )
endfunction()

function(run_GoogleTestXML DISCOVERY_MODE)
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/GoogleTestXML-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  if(NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
  endif()
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake_with_options(GoogleTestXML -DCMAKE_GTEST_DISCOVER_TESTS_DISCOVERY_MODE=${DISCOVERY_MODE})

  run_cmake_command(GoogleTestXML-discovery
  ${CMAKE_COMMAND}
  --build .
  --config Debug
  --target xml_output
  )

  run_cmake_command(GoogleTestXML-result
  ${CMAKE_CTEST_COMMAND}
  -C Debug
  -R GoogleTestXML
  --no-label-summary
  )
endfunction()

function(run_GoogleTest_discovery_timeout DISCOVERY_MODE)
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/GoogleTest-discovery-timeout)
  set(RunCMake_TEST_NO_CLEAN 1)
  if(NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
  endif()
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake_with_options(GoogleTestDiscoveryTimeout -DDISCOVERY_MODE=${DISCOVERY_MODE})

  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(GoogleTest-discovery-${DISCOVERY_MODE}-timeout-build
    ${CMAKE_COMMAND}
    --build .
    --config Debug
    --target discovery_timeout_test
  )
  set(RunCMake_TEST_OUTPUT_MERGE 0)

  run_cmake_command(GoogleTest-discovery-${DISCOVERY_MODE}-timeout-test
    ${CMAKE_CTEST_COMMAND}
    -C Debug
    -R discovery_timeout_test
    --no-label-sumary
  )
endfunction()

function(run_GoogleTest_discovery_multi_config)
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/GoogleTest-discovery-multi-config)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(GoogleTestDiscoveryMultiConfig)

  run_cmake_command(GoogleTest-build-release
    ${CMAKE_COMMAND}
    --build .
    --config Release
    --target configuration_gtest
  )
  run_cmake_command(GoogleTest-build-debug
    ${CMAKE_COMMAND}
    --build .
    --config Debug
    --target configuration_gtest
  )
  run_cmake_command(GoogleTest-configuration-release
    ${CMAKE_CTEST_COMMAND}
    -C Release
    -L CONFIG
    -N
  )
  run_cmake_command(GoogleTest-configuration-debug
    ${CMAKE_CTEST_COMMAND}
    -C Debug
    -L CONFIG
    -N
  )

endfunction()

foreach(DISCOVERY_MODE POST_BUILD PRE_TEST)
  message("Testing ${DISCOVERY_MODE} discovery mode via CMAKE_GTEST_DISCOVER_TESTS_DISCOVERY_MODE global override...")
  run_GoogleTest(${DISCOVERY_MODE})
  run_GoogleTestXML(${DISCOVERY_MODE})
  message("Testing ${DISCOVERY_MODE} discovery mode via DISCOVERY_MODE option...")
  run_GoogleTest_discovery_timeout(${DISCOVERY_MODE})
endforeach()

if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  message("Testing PRE_TEST discovery multi configuration...")
  run_GoogleTest_discovery_multi_config()
endif()
