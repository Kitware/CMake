enable_language(C)

add_library(lib1 STATIC)
target_sources(lib1 PUBLIC FILE_SET a TYPE SOURCES FILES lib2.c)

set_property(FILE_SET a TARGET lib1 PROPERTY COMPILE_DEFINITIONS LIB1_A)
set_property(FILE_SET a TARGET lib1 PROPERTY INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/subdir1")

set_property(FILE_SET a TARGET lib1 PROPERTY INTERFACE_COMPILE_DEFINITIONS INTERFACE_LIB1_A)
set_property(FILE_SET a TARGET lib1 PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/subdir2")

add_library(lib2 STATIC lib1.c)
target_link_libraries(lib2 INTERFACE lib1)

add_executable(main main.c)
target_link_libraries(main PRIVATE lib2)
target_compile_definitions(main PRIVATE CONSUMER)
