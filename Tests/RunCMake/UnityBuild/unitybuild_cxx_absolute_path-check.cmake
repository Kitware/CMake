set(unitybuild_cxx "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/tgt.dir/Unity/unity_0_cxx.cxx")
if(NOT EXISTS "${unitybuild_cxx}")
  set(RunCMake_TEST_FAILED "Generated unity source file ${unitybuild_cxx} does not exist.")
  return()
endif()

string(JOIN ".*" EXPECTED_UNITY_FILE_CONTENT
  [[#include "([A-Za-z]:)?/[^"]*/Tests/RunCMake/UnityBuild/unitybuild_cxx_absolute_path-build/s1\.cxx"]]
  [[#include "([A-Za-z]:)?/[^"]*/Tests/RunCMake/UnityBuild/unitybuild_cxx_absolute_path-build/s2\.cxx"]]
  [[#include "([A-Za-z]:)?/[^"]*/Tests/RunCMake/UnityBuild/unitybuild_cxx_absolute_path-build/s3\.cxx"]]
)

file(STRINGS ${unitybuild_cxx} unitybuild_cxx_strings)
if(NOT unitybuild_cxx_strings MATCHES "${EXPECTED_UNITY_FILE_CONTENT}")
  set(RunCMake_TEST_FAILED "Generated unity file ${unitybuild_cxx} doesn't contain absolute paths")
  return()
endif()
