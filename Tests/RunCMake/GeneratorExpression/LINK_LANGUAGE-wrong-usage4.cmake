
enable_language(C)

add_library(libC empty.c)

add_executable(empty empty.c)
target_link_libraries(empty PRIVATE lib$<LINK_LANGUAGE>)
