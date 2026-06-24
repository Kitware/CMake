
enable_language(C)

add_library(foo STATIC)

add_subdirectory(subdir1)

target_sources(foo PRIVATE FILE_SET fs TYPE simple FILES file.i)
