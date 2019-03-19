add_test(NAME FindX11.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindX11/Test"
  "${CMake_BINARY_DIR}/Tests/FindX11/Test"
  ${build_generator_args}
  --build-project TestFindX11
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
