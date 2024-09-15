
find_package_lib1()

add_library(lib2 STATIC ../lib2.c)
target_link_libraries(lib2 PUBLIC lib1::lib1)
