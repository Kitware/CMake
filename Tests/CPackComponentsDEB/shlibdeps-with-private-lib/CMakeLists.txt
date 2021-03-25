add_library(myprivatelib SHARED myprivatelib.cpp)
set_target_properties( myprivatelib PROPERTIES
    VERSION 1.2.3
    SOVERSION 1
)
