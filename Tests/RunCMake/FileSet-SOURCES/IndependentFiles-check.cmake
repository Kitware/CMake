if (EXISTS "${RunCMake_TEST_BINARY_DIR}/gen.c")
  list(APPEND RunCMake_TEST_FAILED
    "The `gen.c` file should not be generated to compile `unrelated`'s object")
endif ()
