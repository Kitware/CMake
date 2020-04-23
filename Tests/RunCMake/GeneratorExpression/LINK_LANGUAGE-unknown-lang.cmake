
enable_language(C)
add_executable(empty empty.c)
target_link_options(empty PRIVATE $<$<LINK_LANGUAGE:CXX>:$<TARGET_EXISTS:too,many,parameters>>)
