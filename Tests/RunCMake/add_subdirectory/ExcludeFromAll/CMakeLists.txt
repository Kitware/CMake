project(ExcludeFromAllSub NONE)

add_subdirectory(SubSub EXCLUDE_FROM_ALL)

add_library(bar STATIC EXCLUDE_FROM_ALL bar.cpp)

add_library(zot STATIC zot.cpp)

add_library(foo STATIC foo.cpp)
target_include_directories(foo PUBLIC .)

add_library(subinc STATIC subinc.cpp)
set_target_properties(subinc PROPERTIES EXCLUDE_FROM_ALL OFF)
