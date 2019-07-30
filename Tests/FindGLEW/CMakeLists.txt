add_test(NAME FindGLEW.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindGLEW/Test"
  "${CMake_BINARY_DIR}/Tests/FindGLEW/Test"
  ${build_generator_args}
  --build-project TestFindGLEW
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
