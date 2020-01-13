
enable_language(C)

cmake_policy (SET CMP0101 OLD)

add_executable (CMP0101_OLD CMP0101.c)
target_compile_options (CMP0101_OLD PRIVATE -UBEFORE_KEYWORD)
target_compile_options (CMP0101_OLD BEFORE PRIVATE -DBEFORE_KEYWORD)


cmake_policy (SET CMP0101 NEW)

add_executable (CMP0101_NEW CMP0101.c)
target_compile_options (CMP0101_NEW PRIVATE -UBEFORE_KEYWORD)
target_compile_options (CMP0101_NEW BEFORE PRIVATE -DBEFORE_KEYWORD)
