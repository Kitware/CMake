set(foo_pch_header "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/foo.dir/cmake_pch.h")
set(foobar_pch_header "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/foobar.dir/cmake_pch.h")
if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(foo_pch_header "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/foo.dir/Debug/cmake_pch.h")
  set(foobar_pch_header "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/foobar.dir/Debug/cmake_pch.h")
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

if (NOT foo_pch_header_strings MATCHES ";#include \"[^\"]*PrecompileHeaders/include/foo.h\";#include \"foo2.h\";#include <stdio.h>;#include \"string.h\"(;|$)")
  set(RunCMake_TEST_FAILED "Generated foo pch header\n  ${foo_pch_header}\nhas bad content:\n  ${foo_pch_header_strings}")
  return()
endif()

file(STRINGS ${foobar_pch_header} foobar_pch_header_strings)

if (NOT foobar_pch_header_strings MATCHES ";#include \"[^\"]*PrecompileHeaders/include/foo.h\";#include \"foo2.h\";#include <stdio.h>;#include \"string.h\";#include \"[^\"]*PrecompileHeaders/include/bar.h\"(;|$)")
  set(RunCMake_TEST_FAILED "Generated foobar pch header\n  ${foobar_pch_header}\nhas bad content:\n  ${foobar_pch_header_strings}")
  return()
endif()
