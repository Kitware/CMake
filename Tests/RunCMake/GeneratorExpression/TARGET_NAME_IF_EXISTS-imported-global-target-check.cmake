file(READ "${RunCMake_TEST_BINARY_DIR}/TARGET_NAME_IF_EXISTS-generated-imported-global.txt" content)

if(NOT content STREQUAL importedGlobalTarget)
  set(RunCMake_TEST_FAILED "actual content:\n ${content}\nbut expected [[importedGlobalTarget]]")
endif()
