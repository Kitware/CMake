add_test(NAME FindLibArchive.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindLibArchive/Test"
  "${CMake_BINARY_DIR}/Tests/FindLibArchive/Test"
  ${build_generator_args}
  --build-project TestFindLibArchive
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
