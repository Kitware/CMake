add_library(subsub STATIC subsub.cpp)
add_library(subsubinc STATIC subsub.cpp)
set_property(TARGET subsubinc PROPERTY EXCLUDE_FROM_ALL OFF)
