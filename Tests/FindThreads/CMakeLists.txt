add_test(NAME FindThreads.C-only COMMAND ${CMAKE_CTEST_COMMAND}
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindThreads/C-only"
  "${CMake_BINARY_DIR}/Tests/FindThreads/C-only"
  ${build_generator_args}
  --build-project FindThreads_C-only
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V
  )
