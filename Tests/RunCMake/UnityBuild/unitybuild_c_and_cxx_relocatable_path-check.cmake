set(unitybuild_c "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/tgt.dir/Unity/unity_0_c.c")
if(NOT EXISTS "${unitybuild_c}")
  set(RunCMake_TEST_FAILED "Generated unity source files ${unitybuild_c} does not exist.")
  return()
endif()

string(JOIN ".*" EXPECTED_UNITY_FILE_CONTENT
  [[#include "\.\./\.\./\.\./s1\.c"]]
  [[#include "\.\./\.\./\.\./s2\.c"]]
  [[#include "\.\./\.\./\.\./s3\.c"]]
)

file(STRINGS ${unitybuild_c} unitybuild_c_strings)
if(NOT unitybuild_c_strings MATCHES "${EXPECTED_UNITY_FILE_CONTENT}")
  set(RunCMake_TEST_FAILED "Generated unity file ${unitybuild_c} doesn't contain relative paths")
  return()
endif()

set(unitybuild_cxx "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/tgt.dir/Unity/unity_0_cxx.cxx")
if(NOT EXISTS "${unitybuild_cxx}")
  set(RunCMake_TEST_FAILED "Generated unity source files ${unitybuild_cxx} does not exist.")
  return()
endif()

string(JOIN ".*" EXPECTED_UNITY_FILE_CONTENT_CXX
  [[#include "\.\./\.\./\.\./s1\.cxx"]]
  [[#include "\.\./\.\./\.\./s2\.cxx"]]
  [[#include "\.\./\.\./\.\./s3\.cxx"]]
)

file(STRINGS ${unitybuild_cxx} unitybuild_cxx_strings)
if(NOT unitybuild_cxx_strings MATCHES "${EXPECTED_UNITY_FILE_CONTENT_CXX}")
  set(RunCMake_TEST_FAILED "Generated unity file ${unitybuild_cxx} doesn't contain relative paths")
  return()
endif()
