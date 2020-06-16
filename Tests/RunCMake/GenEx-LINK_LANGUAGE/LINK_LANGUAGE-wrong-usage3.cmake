
enable_language(C)
add_executable(empty empty.c)
target_include_directories(empty PRIVATE $<$<LINK_LANGUAGE:C>:/DIR>)
