add_test(NAME FindBoost.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindBoost/Test"
  "${CMake_BINARY_DIR}/Tests/FindBoost/Test"
  ${build_generator_args}
  --build-project TestFindBoost
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )

add_test(NAME FindBoost.TestFail COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindBoost/TestFail"
  "${CMake_BINARY_DIR}/Tests/FindBoost/TestFail"
  ${build_generator_args}
  --build-project TestFailFindBoost
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )

set_tests_properties(FindBoost.TestFail PROPERTIES
  WILL_FAIL ON
  PASS_REGULAR_EXPRESSION "Could NOT find Boost (missing: foobar)")

add_test(NAME FindBoost.TestHeaders COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindBoost/TestHeaders"
  "${CMake_BINARY_DIR}/Tests/FindBoost/TestHeaders"
  ${build_generator_args}
  --build-project TestFindBoostHeaders
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )

if (CMake_TEST_FindBoost_Python)
  add_test(NAME FindBoost.TestPython COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindBoost/TestPython"
    "${CMake_BINARY_DIR}/Tests/FindBoost/TestPython"
    ${build_generator_args}
    --build-project TestFindBoostPython
    --build-options ${build_options}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
endif ()
