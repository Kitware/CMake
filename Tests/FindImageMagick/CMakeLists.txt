add_test(NAME FindImageMagick.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindImageMagick/Test"
  "${CMake_BINARY_DIR}/Tests/FindImageMagick/Test"
  ${build_generator_args}
  --build-project TestFindImageMagick
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
