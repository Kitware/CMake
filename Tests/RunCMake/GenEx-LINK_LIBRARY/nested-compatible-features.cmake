enable_language(C)

set(CMAKE_C_LINK_LIBRARY_USING_feat1_SUPPORTED TRUE)
set(CMAKE_C_LINK_LIBRARY_USING_feat1 "<LIBRARY>")

add_library(dep1 SHARED empty.c)

add_library(dep2 SHARED empty.c)

add_library(lib SHARED empty.c)
target_link_libraries(lib PRIVATE "$<LINK_LIBRARY:feat1,dep1,$<LINK_LIBRARY:feat1,dep2>>")
