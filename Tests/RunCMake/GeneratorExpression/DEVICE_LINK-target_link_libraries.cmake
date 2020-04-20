
enable_language(C)

add_library(lib empty.c)

add_executable(empty empty.c)
target_link_libraries(empty PRIVATE $<DEVICE_LINK:lib>)
