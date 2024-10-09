add_test(NAME FindBacktrace.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindBacktrace/Test"
  "${CMake_BINARY_DIR}/Tests/FindBacktrace/Test"
  ${build_generator_args}
  --build-project TestFindBacktrace
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
