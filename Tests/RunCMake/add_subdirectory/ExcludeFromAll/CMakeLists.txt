add_library(bar STATIC bar.cpp)

add_library(foo STATIC foo.cpp)

add_library(baz STATIC foo.cpp)
set_target_properties(baz PROPERTIES EXCLUDE_FROM_ALL OFF)

target_include_directories(foo PUBLIC .)
