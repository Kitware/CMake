enable_language(C)

set(CMAKE_C_LINK_LIBRARY_USING_feat1_SUPPORTED TRUE)
set(CMAKE_C_LINK_LIBRARY_USING_feat1 "<LIBRARY>")

set(CMAKE_C_LINK_LIBRARY_USING_feat2_SUPPORTED TRUE)
set(CMAKE_C_LINK_LIBRARY_USING_feat2 "<LIBRARY>")

add_library(dep1 SHARED empty.c)

add_library(dep2 SHARED empty.c)
target_link_libraries(dep2 PUBLIC "$<LINK_LIBRARY:feat1,dep1>")

add_library(lib SHARED empty.c)
target_link_libraries(lib PRIVATE dep1 dep2)
