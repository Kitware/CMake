
enable_language(C)
add_executable(empty empty.c)
target_compile_options(empty PRIVATE $<$<LINK_LANG_AND_ID:C,GNU>:-OPT>)
