
enable_language(C)

add_library(bar empty.c)

source_group(foo FILE_SETS bar TARGET bar)
