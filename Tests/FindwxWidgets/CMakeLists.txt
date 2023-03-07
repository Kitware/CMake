add_test(NAME FindwxWidgets.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindwxWidgets/Test"
  "${CMake_BINARY_DIR}/Tests/FindwxWidgets/Test"
  ${build_generator_args}
  --build-project TestFindwxWidgets
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
