file(GLOB_RECURSE ok_files "${RunCMake_TEST_BINARY_DIR}/install/*ok.txt")
if (ok_files)
  set(RunCMake_TEST_FAILED
    "comp_ok was installed after comp_fail failed; serial fail-fast not honored: ${ok_files}")
endif ()
