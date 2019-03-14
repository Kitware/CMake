add_test(NAME FindLibinput.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindLibinput/Test"
  "${CMake_BINARY_DIR}/Tests/FindLibinput/Test"
  ${build_generator_args}
  --build-project TestFindLibinput
  --build-options ${build_options}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
