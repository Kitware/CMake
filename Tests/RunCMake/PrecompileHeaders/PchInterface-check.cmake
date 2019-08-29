if (NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(foo_pch_header "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/foo.dir/CMakeFiles/foo.dir/cmake_pch.h")
  set(foobar_pch_header "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/foobar.dir/CMakeFiles/foobar.dir/cmake_pch.h")
else()
  set(foo_pch_header "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/foo.dir/cmake_pch.h")
  set(foobar_pch_header "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/foobar.dir/cmake_pch.h")
endif()

if (NOT EXISTS ${foo_pch_header})
  set(RunCMake_TEST_FAILED "Generated foo pch header ${foo_pch_header} does not exist")
  return()
endif()

if (NOT EXISTS ${foobar_pch_header})
  set(RunCMake_TEST_FAILED "Generated foobar pch header ${foobar_pch_header} does not exist")
  return()
endif()

file(STRINGS ${foo_pch_header} foo_pch_header_strings)

if (NOT "#include \"foo.h\"" IN_LIST foo_pch_header_strings OR
    NOT "#include <stdio.h>" IN_LIST foo_pch_header_strings OR
    NOT "#include \"string.h\"" IN_LIST foo_pch_header_strings)
  set(RunCMake_TEST_FAILED "Generated foo pch header ${foo_pch_header} has bad content")
  return()
endif()

file(STRINGS ${foobar_pch_header} foobar_pch_header_strings)

if (NOT "#include \"foo.h\"" IN_LIST foobar_pch_header_strings OR
    NOT "#include <stdio.h>" IN_LIST foobar_pch_header_strings OR
    NOT "#include \"string.h\"" IN_LIST foobar_pch_header_strings OR
    NOT "#include \"bar.h\"" IN_LIST foobar_pch_header_strings)
  set(RunCMake_TEST_FAILED "Generated foobar pch header ${foobar_pch_header} has bad content")
  return()
endif()
