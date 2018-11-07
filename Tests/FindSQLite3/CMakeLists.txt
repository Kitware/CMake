add_test(NAME FindSQLite3.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindSQLite3/Test"
  "${CMake_BINARY_DIR}/Tests/FindSQLite3/Test"
  ${build_generator_args}
  --build-project TestFindSQLite3
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
