add_test(NAME FindLibLZMA.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindLibLZMA/Test"
  "${CMake_BINARY_DIR}/Tests/FindLibLZMA/Test"
  ${build_generator_args}
  --build-project TestFindLibLZMA
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
