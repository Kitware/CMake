enable_language(C CXX)

add_executable(app-C empty.c)
add_executable(app-CXX empty.cxx)
add_library(lib-C SHARED empty.c)
add_library(lib-CXX SHARED empty.cxx)

set_property(TARGET app-C PROPERTY VERSION 0.1)
set_property(TARGET app-CXX PROPERTY VERSION 1.0)
set_property(TARGET lib-C PROPERTY VERSION 65535.65535)
set_property(TARGET lib-CXX PROPERTY VERSION "")
