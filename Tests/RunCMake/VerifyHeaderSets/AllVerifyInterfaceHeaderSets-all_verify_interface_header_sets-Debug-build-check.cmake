# A custom command is used to copy the header file from the source directory to
# the binary directory. If the verification target was built, the custom
# command should have been executed, and the file should be present in the
# binary directory.
if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/dir1/lib1.h")
  string(APPEND RunCMake_TEST_FAILED "${RunCMake_TEST_BINARY_DIR}/dir1/lib1.h should exist but it does not\n")
endif()
if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/dir2/lib2.h")
  string(APPEND RunCMake_TEST_FAILED "${RunCMake_TEST_BINARY_DIR}/dir2/lib2.h should exist but it does not\n")
endif()
