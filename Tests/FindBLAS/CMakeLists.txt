add_test(NAME FindBLAS.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindBLAS/Test"
  "${CMake_BINARY_DIR}/Tests/FindBLAS/Test"
  ${build_generator_args}
  --build-project TestFindBLAS
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
