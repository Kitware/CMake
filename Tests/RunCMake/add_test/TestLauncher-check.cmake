set(testfile "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake")
if(EXISTS "${testfile}")
  file(READ "${testfile}" testfile_contents)
else()
  message(FATAL_ERROR "Could not find expected CTestTestfile.cmake.")
endif()

set(error_details "There is a problem with generated test file: ${testfile}")

if(testfile_contents MATCHES "add_test[(]DoesNotUseTestLauncher [^\n]+pseudo_test_launcher[^\n]+\n")
  message(SEND_ERROR "Used test launcher when it should not be used. ${error_details}")
endif()

if(NOT testfile_contents MATCHES "add_test[(]UsesTestLauncher [^\n]+pseudo_test_launcher[^\n]+\n")
  message(SEND_ERROR "Did not use test launcher when it should be used. ${error_details}")
endif()

if(testfile_contents MATCHES "add_test[(]DoesNotUseTestLauncherWithGenex [^\n]+pseudo_test_launcher[^\n]+\n")
  message(SEND_ERROR "Used test launcher when it should not be used. ${error_details}")
endif()

if(NOT testfile_contents MATCHES "add_test[(]UsesTestLauncherWithExecTargetFromSubdirAddedWithoutGenex [^\n]+pseudo_test_launcher[^\n]+\n")
  message(SEND_ERROR "Did not use test launcher when it should be used. ${error_details}")
endif()

if(testfile_contents MATCHES "add_test[(]DoesNotUseTestLauncherWithExecTargetFromSubdirAddedWithGenex [^\n]+pseudo_test_launcher[^\n]+\n")
  message(SEND_ERROR "Used test launcher when it should not be used. ${error_details}")
endif()
