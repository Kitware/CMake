add_test(NAME FindASPELL.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindASPELL/Test"
  "${CMake_BINARY_DIR}/Tests/FindASPELL/Test"
  ${build_generator_args}
  --build-project TestFindASPELL
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
