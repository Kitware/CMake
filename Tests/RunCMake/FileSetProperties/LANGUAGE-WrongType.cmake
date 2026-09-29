add_library(foo STATIC foo.c)
target_sources(foo PRIVATE FILE_SET modules TYPE CXX_MODULES)
set_property(FILE_SET modules TARGET foo PROPERTY LANGUAGE CXX)
