include(RunCMake)

# Link should succeed
block()
  set(libdir ${RunCMake_BINARY_DIR}/TestLib-build/TestLib/lib)
  run_cmake(TestLib)
  run_cmake_with_options(TestApp "-DCMAKE_C_STANDARD_LINK_DIRECTORIES=${libdir}")
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(TestLib ${CMAKE_COMMAND} --build .)
  run_cmake_command(TestAppGood ${CMAKE_COMMAND} --build ../TestApp-build)
endblock()

# Link should fail
block()
  run_cmake(TestLib)
  run_cmake(TestApp)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(TestLib ${CMAKE_COMMAND} --build .)
  run_cmake_command(TestAppBad ${CMAKE_COMMAND} --build ../TestApp-build)
endblock()
