enable_language(C)

set(CMAKE_C_LINK_LIBRARY_USING_feat1_SUPPORTED TRUE)
set(CMAKE_C_LINK_LIBRARY_USING_feat1 "<LIBRARY>")

set(CMAKE_C_LINK_LIBRARY_USING_feat2_SUPPORTED TRUE)
set(CMAKE_C_LINK_LIBRARY_USING_feat2 "<LIBRARY>")

add_library(dep1 SHARED empty.c)

add_library(dep2 SHARED empty.c)
target_link_libraries(dep2 PRIVATE "$<LINK_LIBRARY:feat1,dep1>")

add_library(dep3 SHARED empty.c)
target_link_libraries(dep3 PUBLIC dep2)

add_library(lib1 SHARED empty.c)
target_link_libraries(lib1 PRIVATE "$<LINK_LIBRARY:feat2,dep1,dep2>")

add_library(lib2 SHARED empty.c)
target_link_libraries(lib2 PRIVATE "$<LINK_LIBRARY:DEFAULT,dep2,dep3>")
