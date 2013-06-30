
project(CMP0022-NOWARN-static)

add_library(foo STATIC empty.cpp)
add_library(bar STATIC empty.cpp)
add_library(bat STATIC empty.cpp)
target_link_libraries(foo bar)
target_link_libraries(bar bat)
