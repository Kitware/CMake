add_test(NAME FindGnuTLS.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindGnuTLS/Test"
  "${CMake_BINARY_DIR}/Tests/FindGnuTLS/Test"
  ${build_generator_args}
  --build-project TestFindGnuTLS
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
