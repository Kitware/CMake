set(foobar_pch_h_header "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/foobar.dir/cmake_pch.h")
set(foobar_pch_hxx_header "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/foobar.dir/cmake_pch.hxx")

if (NOT EXISTS ${foobar_pch_h_header})
  set(RunCMake_TEST_FAILED "Generated foobar C pch header ${foobar_pch_h_header} does not exist")
  return()
endif()

if (NOT EXISTS ${foobar_pch_hxx_header})
  set(RunCMake_TEST_FAILED "Generated foobar C++ pch header ${foobar_pch_hxx_header} does not exist")
  return()
endif()

file(STRINGS ${foobar_pch_h_header} foobar_pch_h_header_strings)

if (NOT foobar_pch_h_header_strings MATCHES ";#include <stddef.h>(;|$)")
  set(RunCMake_TEST_FAILED "Generated foo pch header\n  ${foobar_pch_h_header}\nhas bad content:\n  ${foobar_pch_h_header_strings}")
  return()
endif()

file(STRINGS ${foobar_pch_hxx_header} foobar_pch_hxx_header_strings)

if (NOT foobar_pch_hxx_header_strings MATCHES ";#include <cstddef>(;|$)")
  set(RunCMake_TEST_FAILED "Generated foo pch header\n  ${foobar_pch_hxx_header}\nhas bad content:\n  ${foobar_pch_hxx_header_strings}")
  return()
endif()
