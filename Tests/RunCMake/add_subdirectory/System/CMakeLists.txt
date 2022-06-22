project(SystemSub NONE)

add_subdirectory(SubSub1 SYSTEM)
add_subdirectory(SubSub2)

add_library(bar STATIC bar.cpp)

add_library(foo STATIC foo.cpp)
set_target_properties(foo PROPERTIES SYSTEM OFF)

add_executable(zot zot.cpp)
