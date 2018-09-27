add_test(NAME FindFontconfig.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindFontconfig/Test"
  "${CMake_BINARY_DIR}/Tests/FindFontconfig/Test"
  ${build_generator_args}
  --build-project TestFindFontconfig
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
