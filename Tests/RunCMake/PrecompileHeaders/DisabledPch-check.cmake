set(foo_pch_header "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/foo.dir/cmake_pch.h")
set(foobar_pch_header "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/foobar.dir/cmake_pch.h")

if (NOT EXISTS ${foo_pch_header})
  set(RunCMake_TEST_FAILED "Generated foo pch header ${foo_pch_header} does not exist")
  return()
endif()

if (EXISTS ${foobar_pch_header})
  set(RunCMake_TEST_FAILED "Generated foobar pch header ${foobar_pch_header} should not exist")
  return()
endif()
