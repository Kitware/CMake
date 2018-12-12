add_library(lib1 STATIC depends_obj0.c)
add_library(lib2 OBJECT a.c)
target_link_libraries(lib2 PRIVATE lib1)

add_executable(test exe2.c)

target_link_libraries(test PUBLIC lib2)
