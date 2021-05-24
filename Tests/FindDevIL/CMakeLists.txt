add_test(NAME FindDevIL.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindDevIL/Test"
  "${CMake_BINARY_DIR}/Tests/FindDevIL/Test"
  ${build_generator_args}
  --build-project TestFindDevIL
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
