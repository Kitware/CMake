add_test(NAME FindLibXslt.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindLibXslt/Test"
  "${CMake_BINARY_DIR}/Tests/FindLibXslt/Test"
  ${build_generator_args}
  --build-project TestFindLibXslt
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
)
