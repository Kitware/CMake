
cmake_policy(VERSION 3.1)

file(STRINGS "${RunCMake_TEST_BINARY_DIR}/CMP0122-library-name.txt" prefixes)

list(GET prefixes 0 std_prefix)
list(GET prefixes 1 lib_prefix)
if (NOT std_prefix STREQUAL lib_prefix)
  string (APPEND RunCMake_TEST_FAILED "\nFound prefix: '${lib_prefix}', expected: '${std_prefix}'.")
endif()
