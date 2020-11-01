set(unitybuild_a_c "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/tgt.dir/Unity/unity_a_c.c")
if(NOT EXISTS "${unitybuild_a_c}")
  set(RunCMake_TEST_FAILED "Generated unity source files ${unitybuild_a_c} does not exist.")
  return()
else()
  #verify that the 4 c file is part of this grouping and therefore UNITY_BUILD_BATCH_SIZE
  #was ignored
  file(STRINGS ${unitybuild_a_c} unitybuild_a_c_strings)
  string(REGEX MATCH ".*#include.*s1.c.*#include.*s2.c.*#include.*s3.c.*#include.*s4.c.*" matched_code ${unitybuild_a_c_strings})
  if(NOT matched_code)
    set(RunCMake_TEST_FAILED "Generated unity file doesn't include expected source files")
    return()
  endif()
endif()

set(unitybuild_b_c "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/tgt.dir/Unity/unity_b_c.c")
if(NOT EXISTS "${unitybuild_b_c}")
  set(RunCMake_TEST_FAILED "Generated unity source files ${unitybuild_b_c} does not exist.")
  return()
endif()


set(unitybuild_a_cxx "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/tgt.dir/Unity/unity_a_cxx.cxx")
if(NOT EXISTS "${unitybuild_a_cxx}")
  set(RunCMake_TEST_FAILED "Generated unity source files ${unitybuild_a_cxx} does not exist.")
  return()
else()
  #verify that the 4 cxx file are part of this grouping and therefore UNITY_BUILD_BATCH_SIZE
  #was ignored
  file(STRINGS ${unitybuild_a_cxx} unitybuild_a_cxx_strings)
  string(REGEX MATCH ".*#include.*s1.cxx.*#include.*s2.cxx.*#include.*s3.cxx.*#include.*s4.cxx.*" matched_code ${unitybuild_a_cxx_strings})
  if(NOT matched_code)
    set(RunCMake_TEST_FAILED "Generated unity file doesn't include expected source files")
    return()
  endif()
endif()

set(unitybuild_b_cxx "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/tgt.dir/Unity/unity_b_cxx.cxx")
if(NOT EXISTS "${unitybuild_b_cxx}")
  set(RunCMake_TEST_FAILED "Generated unity source files ${unitybuild_b_cxx} does not exist.")
  return()
endif()
