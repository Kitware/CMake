# A custom command is used to copy the header file from the source directory to
# the binary directory. If the verification target was built, the custom
# command should have been executed, and the file should be present in the
# binary directory.
if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/dir3/lib3.h")
  string(APPEND RunCMake_TEST_FAILED "${RunCMake_TEST_BINARY_DIR}/dir3/lib3.h should exist but it does not\n")
endif()
if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/dir4/lib4.h")
  string(APPEND RunCMake_TEST_FAILED "${RunCMake_TEST_BINARY_DIR}/dir4/lib4.h should exist but it does not\n")
endif()
