
enable_language(C)

add_library(base STATIC base.c unref.c)
target_compile_definitions(base PUBLIC STATIC_BASE)

add_library(lib SHARED lib.c)
target_link_libraries(lib PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,base>")

add_executable(main main.c)
target_link_libraries(main PRIVATE lib)
