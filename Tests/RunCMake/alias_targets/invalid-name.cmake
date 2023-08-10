cmake_policy(SET CMP0037 OLD)
enable_language(CXX)

add_library(foo empty.cpp)

add_library(invalid$name ALIAS foo)
