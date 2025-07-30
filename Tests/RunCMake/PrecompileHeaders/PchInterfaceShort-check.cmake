set(foobar_pch_header "${RunCMake_TEST_BINARY_DIR}/.o/ff32d702/cmake_pch.h")
if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(foobar_pch_header "${RunCMake_TEST_BINARY_DIR}/.o/ff32d702/Debug/cmake_pch.h")
endif()

if (NOT EXISTS ${foobar_pch_header})
  set(RunCMake_TEST_FAILED "Generated foobar pch header ${foobar_pch_header} does not exist")
  return()
endif()
