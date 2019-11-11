set(main_pch_header "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/main.dir/cmake_pch.hxx")

file(STRINGS ${main_pch_header} main_pch_header_strings)
string(REGEX MATCH "#pragma warning\\(push, 0\\).*#include.*pch.h.*#pragma warning\\(pop\\)" matched_code ${main_pch_header_strings})
if(NOT matched_code)
  set(RunCMake_TEST_FAILED "Generated pch file doesn't include expected prologue and epilogue code")
  return()
endif()
