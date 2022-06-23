add_test(NAME FindOpenAL.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindOpenAL/Test"
  "${CMake_BINARY_DIR}/Tests/FindOpenAL/Test"
  ${build_generator_args}
  --build-project TestFindOpenAL
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
