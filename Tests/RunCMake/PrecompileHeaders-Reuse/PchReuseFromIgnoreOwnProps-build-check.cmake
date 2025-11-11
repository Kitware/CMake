if(MSVC OR (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND CMAKE_C_SIMULATE_ID STREQUAL "MSVC"))
  find_file(pdb_file
    NAMES NOT_USED.pdb
    PATHS "${RunCMake_TEST_BINARY_DIR}"
    PATH_SUFFIXES NOT_USED NOT_USED_DEBUG)
  if (EXISTS "${pdb_file}")
    list(APPEND RunCMake_TEST_FAILED
      "PDB file was created from properties meant to be ignored")
  endif ()

  if (EXISTS "${RunCMake_TEST_BINARY_DIR}/mycustomdir")
    list(APPEND RunCMake_TEST_FAILED
      "PDB output directory ('mycustomdir') was created (should have used debug variant)")
  endif ()
  if (NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/mycustomdir_debug")
    list(APPEND RunCMake_TEST_FAILED
      "PDB output directory ('mycustomdir_debug') was not created (separate PDBs required)")
  endif ()
endif ()
