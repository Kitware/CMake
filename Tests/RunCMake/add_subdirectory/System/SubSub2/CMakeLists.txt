add_library(subsub2bar STATIC bar.cpp)

add_library(subsub2foo STATIC foo.cpp)
set_target_properties(subsub2foo PROPERTIES SYSTEM OFF)

add_executable(subsub2zot zot.cpp)
