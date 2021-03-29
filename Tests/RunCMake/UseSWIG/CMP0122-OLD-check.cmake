
cmake_policy(VERSION 3.1)

file(STRINGS "${RunCMake_TEST_BINARY_DIR}/CMP0122-library-name.txt" prefixes)

list(GET prefixes 1 lib_prefix)
if (lib_prefix)
  # prefix must be empty
  string (APPEND RunCMake_TEST_FAILED "\nFound unexpected prefix: '${lib_prefix}'.")
endif()
