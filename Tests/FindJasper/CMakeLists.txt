add_test(NAME FindJasper.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindJasper/Test"
  "${CMake_BINARY_DIR}/Tests/FindJasper/Test"
  ${build_generator_args}
  --build-project TestFindJasper
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
