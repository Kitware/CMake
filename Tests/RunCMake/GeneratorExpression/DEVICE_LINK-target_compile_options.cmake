
enable_language(C)
add_executable(empty empty.c)
target_compile_options(empty PRIVATE $<DEVICE_LINK:-OPT>)
