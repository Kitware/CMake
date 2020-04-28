add_test(NAME FindLAPACK.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindLAPACK/Test"
  "${CMake_BINARY_DIR}/Tests/FindLAPACK/Test"
  ${build_generator_args}
  --build-project TestFindLAPACK
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
