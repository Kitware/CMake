add_test(NAME FindTCL.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindTCL/Test"
  "${CMake_BINARY_DIR}/Tests/FindTCL/Test"
  ${build_generator_args}
  --build-project TestFindTCL
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
