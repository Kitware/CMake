file(READ "${RunCMake_TEST_BINARY_DIR}/TARGET_NAME_IF_EXISTS-generated-imported.txt" content)

if(NOT content STREQUAL importedTarget)
  set(RunCMake_TEST_FAILED "actual content:\n ${content}\nbut expected [[importedTarget]]")
endif()
