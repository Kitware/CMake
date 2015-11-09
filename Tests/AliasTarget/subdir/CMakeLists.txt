
add_library(tgt STATIC empty.cpp)
add_library(Sub::tgt ALIAS tgt)
