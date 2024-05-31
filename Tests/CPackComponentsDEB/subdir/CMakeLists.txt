add_library(myotherlib SHARED myotherlib.cpp)
set_target_properties( myotherlib PROPERTIES
    VERSION 1.2.3
    SOVERSION 1
)
