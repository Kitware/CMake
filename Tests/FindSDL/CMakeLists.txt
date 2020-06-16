add_test(NAME FindSDL.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindSDL/Test"
  "${CMake_BINARY_DIR}/Tests/FindSDL/Test"
  ${build_generator_args}
  --build-project TestFindSDL
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
