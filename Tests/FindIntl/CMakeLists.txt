add_test(NAME FindIntl.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindIntl/Test"
  "${CMake_BINARY_DIR}/Tests/FindIntl/Test"
  ${build_generator_args}
  --build-project TestFindIntl
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
