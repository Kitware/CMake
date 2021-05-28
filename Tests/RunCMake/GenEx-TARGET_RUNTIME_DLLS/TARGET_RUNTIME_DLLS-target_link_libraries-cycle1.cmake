enable_language(C)

add_library(lib1 SHARED lib1.c)
target_link_libraries(lib1 PRIVATE $<TARGET_RUNTIME_DLLS:lib1>)
