enable_language(C)

set(CMAKE_LINK_LIBRARY_USING_feature "<LIBRARY>")
set(CMAKE_LINK_LIBRARY_USING_feature_SUPPORTED TRUE)
set(CMAKE_LINK_LIBRARY_feature_PROPERTIES UNICITY=YES,NO)

add_library(dep SHARED empty.c)

add_library(lib SHARED empty.c)
target_link_libraries(lib PRIVATE "$<LINK_LIBRARY:feature,dep>")
