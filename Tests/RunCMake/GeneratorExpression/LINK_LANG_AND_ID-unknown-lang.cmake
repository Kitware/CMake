
enable_language(C)
add_executable(empty empty.c)
target_link_options(empty PRIVATE $<$<LINK_LANG_AND_ID:CXX,GNU>:$<TARGET_EXISTS:too,many,parameters>>)
