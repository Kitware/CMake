
enable_language(C)
add_executable(empty empty.c)
target_compile_options(empty PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,GNU>:$<TARGET_EXISTS:too,many,parameters>>)
