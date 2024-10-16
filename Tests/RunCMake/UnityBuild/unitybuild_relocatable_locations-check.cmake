set(unitybuild_c "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/tgt.dir/Unity/unity_0_c.c")
if(NOT EXISTS "${unitybuild_c}")
  set(RunCMake_TEST_FAILED "Generated unity source file ${unitybuild_c} does not exist.")
  return()
endif()

string(JOIN ".*" EXPECTED_UNITY_FILE_CONTENT
  [[#include "\.\./\.\./\.\./s1\.c"]]
  [[#include "\.\./\.\./\.\./s2\.c"]]
  [[#include "\.\./\.\./\.\./s3\.c"]]
  [[#include "\.\./\.\./\.\./subFolder/sub1\.c"]]
  [[#include "\.\./\.\./\.\./subFolder/sub2\.c"]]
  [[#include "\.\./\.\./\.\./subFolder/sub3\.c"]]
  [[#include "f\.c"]]
  [[#include "relocatable/foo\.c"]]
)

file(STRINGS ${unitybuild_c} unitybuild_c_strings)
if(NOT unitybuild_c_strings MATCHES "${EXPECTED_UNITY_FILE_CONTENT}")
  set(RunCMake_TEST_FAILED "Generated unity file ${unitybuild_c} doesn't contain relative paths")
  return()
endif()
