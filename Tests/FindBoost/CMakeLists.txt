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
