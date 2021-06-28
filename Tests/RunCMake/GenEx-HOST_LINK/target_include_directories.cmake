
enable_language(C)
add_executable(empty empty.c)
target_include_directories(empty PRIVATE $<HOST_LINK:/DIR>)
