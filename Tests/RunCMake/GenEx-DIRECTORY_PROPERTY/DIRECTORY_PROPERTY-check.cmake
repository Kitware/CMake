file(READ "${RunCMake_TEST_BINARY_DIR}/DIRECTORY_PROPERTY-generated.txt" content)

if(NOT content MATCHES "SOURCE_DIR=.+/GenEx-DIRECTORY_PROPERTY"
    OR NOT content MATCHES "SUBDIR_SOURCE_DIR=.+/GenEx-DIRECTORY_PROPERTY/subdir"
    OR NOT content MATCHES "LABELS=L1;L2")
  set(RunCMake_TEST_FAILED "actual content:\n [[${content}]]\nbut expected:\n [[SOURCE_DIR=.+/GenEx-DIRECTORY_PROPERTY\nSUBDIR_SOURCE_DIR=.+/GenEx-DIRECTORY_PROPERTY/subdir\nLABELS=L1;L2]]")
endif()
