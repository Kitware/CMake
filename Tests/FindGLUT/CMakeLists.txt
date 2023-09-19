add_test(NAME FindGLUT.Test COMMAND ${CMAKE_CTEST_COMMAND}
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindGLUT/Test"
  "${CMake_BINARY_DIR}/Tests/FindGLUT/Test"
  ${build_generator_args}
  --build-project TestFindGLUT
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V ${test_options}
  )
