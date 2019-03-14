add_test(NAME FindPostgreSQL.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindPostgreSQL/Test"
  "${CMake_BINARY_DIR}/Tests/FindPostgreSQL/Test"
  ${build_generator_args}
  --build-project TestFindPostgreSQL
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
