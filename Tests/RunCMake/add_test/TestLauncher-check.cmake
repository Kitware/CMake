set(testfile "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake")
if(EXISTS "${testfile}")
  file(READ "${testfile}" testfile_contents)
else()
  set(RunCMake_TEST_FAILED "Could not find expected CTestTestfile.cmake.")
  return()
endif()

set(error_details "There is a problem with generated test file:\n  ${testfile}")

if(testfile_contents MATCHES "add_test[(]DoesNotUseTestLauncher [^$<>\n]+pseudo_emulator[^$<>\n]+\n")
  set(RunCMake_TEST_FAILED "Used test launcher when it should not be used. ${error_details}")
  return()
endif()

if(NOT testfile_contents MATCHES "add_test[(]UsesTestLauncher [^$<>\n]+pseudo_emulator[^$<>\n]+\n")
  set(RunCMake_TEST_FAILED "Did not use test launcher when it should be used. ${error_details}")
  return()
endif()

if(testfile_contents MATCHES "add_test[(]DoesNotUseTestLauncherWithGenex [^$<>\n]+pseudo_emulator[^$<>\n]+\n")
  set(RunCMake_TEST_FAILED "Used test launcher when it should not be used. ${error_details}")
  return()
endif()

if(NOT testfile_contents MATCHES "add_test[(]UsesTestLauncherWithExecTargetFromSubdirAddedWithoutGenex [^$<>\n]+pseudo_emulator[^$<>\n]+\n")
  set(RunCMake_TEST_FAILED "Did not use test launcher when it should be used. ${error_details}")
  return()
endif()

if(testfile_contents MATCHES "add_test[(]DoesNotUseTestLauncherWithExecTargetFromSubdirAddedWithGenex [^$<>\n]+pseudo_emulator[^$<>\n]+\n")
  set(RunCMake_TEST_FAILED "Used test launcher when it should not be used. ${error_details}")
  return()
endif()

if(NOT testfile_contents MATCHES "add_test[(]UsesLocalLauncher [^$<>\n]+local_launcher[^$<>\n]+use_launcher_local[^$<>\n]+\n")
  message(SEND_ERROR "Did not use local test launcher when it should be used. ${error_details}")
endif()
