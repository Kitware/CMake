add_test(NAME FindOpenSP.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindOpenSP/Test"
  "${CMake_BINARY_DIR}/Tests/FindOpenSP/Test"
  ${build_generator_args}
  --build-project TestFindOpenSP
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
