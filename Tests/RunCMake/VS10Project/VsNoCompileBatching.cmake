enable_language(CXX)

add_library(foo foo.cpp)

set(CMAKE_VS_NO_COMPILE_BATCHING ON)
add_library(foo_NB foo.cpp)

add_library(foo_NB_OFF foo.cpp)
set_property(TARGET foo_NB_OFF PROPERTY VS_NO_COMPILE_BATCHING OFF)
