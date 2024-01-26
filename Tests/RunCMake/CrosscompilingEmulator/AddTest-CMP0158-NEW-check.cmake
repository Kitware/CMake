set(testfile "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake")
if(EXISTS "${testfile}")
  file(READ "${testfile}" testfile_contents)
else()
  set(RunCMake_TEST_FAILED "Could not find expected CTestTestfile.cmake.")
  return()
endif()

set(error_details "There is a problem with generated test file:\n  ${testfile}")

if(testfile_contents MATCHES "add_test[(]DoesNotUseEmulator [^$<>\n]+pseudo_emulator[^$<>\n]+\n")
  set(RunCMake_TEST_FAILED "Used emulator when it should not be used. ${error_details}")
  return()
endif()

if(testfile_contents MATCHES "add_test[(]ShouldNotUseEmulator [^$<>\n]+pseudo_emulator[^$<>\n]+\n")
  set(RunCMake_TEST_FAILED "Used emulator when it should be used. ${error_details}")
  return()
endif()

if(testfile_contents MATCHES "add_test[(]DoesNotUseEmulatorWithGenex [^$<>\n]+pseudo_emulator[^$<>\n]+\n")
  set(RunCMake_TEST_FAILED "Used emulator when it should not be used. ${error_details}")
  return()
endif()

if(testfile_contents MATCHES "add_test[(]ShouldNotUseEmulatorWithExecTargetFromSubdirAddedWithoutGenex [^$<>\n]+pseudo_emulator[^$<>\n]+\n")
  set(RunCMake_TEST_FAILED "Used emulator when it should be used. ${error_details}")
  return()
endif()

if(testfile_contents MATCHES "add_test[(]DoesNotUseEmulatorWithExecTargetFromSubdirAddedWithGenex [^$<>\n]+pseudo_emulator[^$<>\n]+\n")
  set(RunCMake_TEST_FAILED "Used emulator when it should not be used. ${error_details}")
  return()
endif()
