set(unitybuild_c "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/tgt.dir/Unity/unity_0_c.c")
file(STRINGS ${unitybuild_c} unitybuild_c_strings)

string(REGEX MATCH "\\/s[1-6].c" matched_files_1_6 ${unitybuild_c_strings})
if(matched_files_1_6)
  set(RunCMake_TEST_FAILED "Generated unity contains s1.c -> s6.c which should have been skipped")
  return()
endif()

string(REGEX MATCH "\\/s[7-8].c" matched_files_7_8 ${unitybuild_c_strings})
if(NOT matched_files_7_8)
  set(RunCMake_TEST_FAILED "Generated unity should have contained s7.c, s8.c!")
  return()
endif()
