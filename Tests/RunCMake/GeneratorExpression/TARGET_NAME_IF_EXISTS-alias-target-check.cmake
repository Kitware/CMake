file(READ "${RunCMake_TEST_BINARY_DIR}/TARGET_NAME_IF_EXISTS-generated-alias.txt" content)

if(NOT content STREQUAL aliasTarget)
  set(RunCMake_TEST_FAILED "actual content:\n ${content}\nbut expected [[aliasTarget]]")
endif()
