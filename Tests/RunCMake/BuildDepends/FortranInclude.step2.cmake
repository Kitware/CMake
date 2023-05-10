file(WRITE "${RunCMake_TEST_BINARY_DIR}/preprocess.inc" "\tPRINT *, 'FortranIncludePreprocess 2'\n")
file(WRITE "${RunCMake_TEST_BINARY_DIR}/no_preprocess.inc" "\tPRINT *, 'FortranIncludeNoPreprocess 2'\n")
