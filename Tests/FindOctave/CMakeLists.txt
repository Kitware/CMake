add_test(NAME FindOctave.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindOctave/Test"
  "${CMake_BINARY_DIR}/Tests/FindOctave/Test"
  ${build_generator_args}
  --build-project TestFindOctave
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
