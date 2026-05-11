set(unitybuild_c "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/fileset.dir/Unity/unity_0_c.c")
file(STRINGS ${unitybuild_c} unitybuild_c_strings)

string(REGEX MATCH "\\/s[0-8].c" matched_files_0_8 ${unitybuild_c_strings})
if(matched_files_0_8)
  set(RunCMake_TEST_FAILED "Generated unity for fileset contains s0.c -> s8.c which should have been skipped")
  return()
endif()

string(REGEX MATCH "\\/s9.c" matched_files_9 ${unitybuild_c_strings})
if(NOT matched_files_9)
  set(RunCMake_TEST_FAILED "Generated unity for fileset should have contained s9.c!")
  return()
endif()


set(unitybuild_c "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/fileset2.dir/Unity/unity_0_c.c")
file(STRINGS ${unitybuild_c} unitybuild_c_strings)

string(REGEX MATCH "\\/s[0-8].c" matched_files_0_8 ${unitybuild_c_strings})
if(matched_files_0_8)
  set(RunCMake_TEST_FAILED "Generated unity for fileset2 contains s0.c -> s8.c which should have been skipped")
  return()
endif()

string(REGEX MATCH "\\/s9.c" matched_files_9 ${unitybuild_c_strings})
if(NOT matched_files_9)
  set(RunCMake_TEST_FAILED "Generated unity for fileset2 should have contained s9.c!")
  return()
endif()
