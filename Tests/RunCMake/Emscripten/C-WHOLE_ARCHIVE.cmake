
enable_language(C)

add_library(base STATIC base.c unref.c)
set_property(TARGET base PROPERTY POSITION_INDEPENDENT_CODE 1)
target_compile_definitions(base PUBLIC STATIC_BASE)

add_library(lib SHARED lib.c)
target_link_libraries(lib PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,base>")
target_link_options(lib PRIVATE -sSIDE_MODULE)

add_executable(main main.c)
set_property(TARGET main PROPERTY POSITION_INDEPENDENT_CODE 1)
target_link_libraries(main PRIVATE lib)
target_link_options(main PRIVATE -sMAIN_MODULE)

add_library(circular1 STATIC circular1.c)
add_library(circular2 STATIC circular2.c)

target_link_libraries(circular1 PRIVATE circular2)
target_link_libraries(circular2 PRIVATE circular1)

add_executable(main_circular main_circular.c)
target_link_libraries(main_circular PRIVATE $<LINK_LIBRARY:WHOLE_ARCHIVE,circular1>)
