enable_language(C)

set(CMAKE_OPTIMIZE_DEPENDENCIES TRUE)
add_library(mylib STATIC mylib.c)
add_library(neverbuild SHARED neverbuild.c)

# Building mylib should not require building neverbuild
target_link_libraries(mylib PRIVATE neverbuild)
set_target_properties(neverbuild PROPERTIES EXCLUDE_FROM_ALL YES)

# Building SharedTop should require SharedBottom to be built
add_library(SharedTop SHARED top.c)
add_library(StaticMiddle STATIC middle.c)
add_library(SharedBottom SHARED bottom.c)
target_link_libraries(SharedTop PRIVATE StaticMiddle)
target_link_libraries(StaticMiddle PRIVATE SharedBottom)
set_target_properties(StaticMiddle SharedBottom PROPERTIES EXCLUDE_FROM_ALL YES)
set_target_properties(StaticMiddle PROPERTIES POSITION_INDEPENDENT_CODE YES)
