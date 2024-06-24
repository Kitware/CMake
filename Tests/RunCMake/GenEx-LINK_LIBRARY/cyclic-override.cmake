enable_language(C)

set(CMAKE_LINK_LIBRARY_USING_feature1 "<LIBRARY>")
set(CMAKE_LINK_LIBRARY_USING_feature1_SUPPORTED TRUE)
set(CMAKE_LINK_LIBRARY_feature1_ATTRIBUTES OVERRIDE=feature2)

set(CMAKE_LINK_LIBRARY_USING_feature2 "<LIBRARY>")
set(CMAKE_LINK_LIBRARY_USING_feature2_SUPPORTED TRUE)
set(CMAKE_LINK_LIBRARY_feature2_ATTRIBUTES OVERRIDE=feature1)

add_library(dep SHARED empty.c)

add_library(lib SHARED empty.c)
target_link_libraries(lib PRIVATE "$<LINK_LIBRARY:feature1,dep>" "$<LINK_LIBRARY:feature2,dep>")
