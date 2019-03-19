add_test(NAME FindGit.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindGit/Test"
  "${CMake_BINARY_DIR}/Tests/FindGit/Test"
  ${build_generator_args}
  --build-project TestFindGit
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
