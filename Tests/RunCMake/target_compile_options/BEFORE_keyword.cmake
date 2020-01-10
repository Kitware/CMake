
add_executable (CMP0101_OLD CMP0101.c)
target_compile_options (main PRIVATE -UBEFORE_KEYWORD)
target_compile_options (main BEFORE PRIVATE -DBEFORE_KEYWORD)

add_executable (CMP0101_NEW CMP0101.c)
target_compile_options (main PRIVATE -UBEFORE_KEYWORD)
target_compile_options (main BEFORE PRIVATE -DBEFORE_KEYWORD)
