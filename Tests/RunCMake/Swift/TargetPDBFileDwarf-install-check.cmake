if(EXISTS "${RunCMake_TEST_BINARY_DIR}/install/bin/SwiftPDBDwarf.pdb")
  string(APPEND RunCMake_TEST_FAILED
    "OPTIONAL install unexpectedly copied SwiftPDBDwarf.pdb\n")
endif()
