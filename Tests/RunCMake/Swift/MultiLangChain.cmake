cmake_policy(SET CMP0157 NEW)
cmake_policy(SET CMP0195 NEW)

enable_language(Swift C)

add_library(L L.swift)
add_library(C C.c)
target_link_libraries(C PRIVATE L)
add_library(E E.swift)
target_link_libraries(E PRIVATE C)
