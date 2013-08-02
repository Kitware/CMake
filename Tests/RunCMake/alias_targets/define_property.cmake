
enable_language(CXX)

add_library(foo empty.cpp)

add_library(alias ALIAS foo)

define_property(TARGET PROPERTY alias bar)
