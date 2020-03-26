if (NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
  return()
endif()

set(foo_pch_header "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/foo.dir/Debug/cmake_pch.h")

if (NOT EXISTS ${foo_pch_header})
  set(RunCMake_TEST_FAILED "Generated foo pch header ${foo_pch_header} does not exist")
  return()
endif()

file(STRINGS ${foo_pch_header} foo_pch_header_strings)

if (NOT foo_pch_header_strings MATCHES ";#include \"[^\"]*PrecompileHeaders/include/foo.h\";#include <stdio.h>(;|$)")
  set(RunCMake_TEST_FAILED "Generated foo pch header\n  ${foo_pch_header}\nhas bad content:\n  ${foo_pch_header_strings}")
  return()
endif()
