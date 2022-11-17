add_library(subsub1bar STATIC bar.cpp)

add_library(subsub1foo STATIC foo.cpp)
set_target_properties(subsub1foo PROPERTIES SYSTEM OFF)

add_executable(subsub1zot zot.cpp)
