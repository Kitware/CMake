add_library(foo STATIC foo.c)
target_sources(foo PRIVATE FILE_SET sources TYPE SOURCES)
set_property(FILE_SET sources TARGET foo PROPERTY LANGUAGE "$<1:CXX>")
