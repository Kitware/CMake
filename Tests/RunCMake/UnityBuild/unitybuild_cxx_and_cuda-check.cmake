set(unitybuild_a_cu "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/tgt.dir/Unity/unity_a_cu.cu")
if(NOT EXISTS "${unitybuild_a_cu}")
  set(RunCMake_TEST_FAILED "Generated unity source files ${unitybuild_a_cu} does not exist.")
  return()
else()
  #verify that the 4 c file is part of this grouping and therefore UNITY_BUILD_BATCH_SIZE
  #was ignored
  file(STRINGS ${unitybuild_a_cu} unitybuild_a_c_strings)
  string(REGEX MATCH ".*#include.*s1.cu.*#include.*s2.cu.*#include.*s3.cu.*#include.*s4.cu.*" matched_code ${unitybuild_a_c_strings})
  if(NOT matched_code)
    set(RunCMake_TEST_FAILED "Generated unity file doesn't include expected source files")
    return()
  endif()
endif()

set(unitybuild_b_cu "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/tgt.dir/Unity/unity_b_cu.cu")
if(NOT EXISTS "${unitybuild_b_cu}")
  set(RunCMake_TEST_FAILED "Generated unity source files ${unitybuild_b_cu} does not exist.")
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
