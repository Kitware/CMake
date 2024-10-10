
enable_language(C)

add_library(base STATIC base.c unref.c)
target_compile_definitions(base PUBLIC STATIC_BASE)

add_library(lib SHARED lib.c)
target_link_libraries(lib PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,base>")

add_executable(main main.c)
target_link_libraries(main PRIVATE lib)


add_library(circular1 STATIC circular1.c)
add_library(circular2 STATIC circular2.c)

target_link_libraries(circular1 PRIVATE circular2)
target_link_libraries(circular2 PRIVATE circular1)

add_executable(main_circular main_circular.c)
target_link_libraries(main_circular PRIVATE $<LINK_LIBRARY:WHOLE_ARCHIVE,circular1>)
