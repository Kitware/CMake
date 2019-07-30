add_test(NAME FindCups.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindCups/Test"
  "${CMake_BINARY_DIR}/Tests/FindCups/Test"
  ${build_generator_args}
  --build-project FindCups
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
