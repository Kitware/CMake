
add_library(foo INTERFACE)
target_sources(foo INTERFACE FILE_SET foo TYPE HEADERS)

set_property(FILE_SET foo TARGET foo)

set_property(FILE_SET foo TARGET foo PROPERTY)
