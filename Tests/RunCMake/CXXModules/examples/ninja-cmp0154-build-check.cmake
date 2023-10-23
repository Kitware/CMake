if (EXISTS "${RunCMake_TEST_BINARY_DIR}/importable.cxx")
  list(APPEND RunCMake_TEST_FAILED
    "The `importable.cxx` file should not be generated to compile `unrelated`'s object")
endif ()
