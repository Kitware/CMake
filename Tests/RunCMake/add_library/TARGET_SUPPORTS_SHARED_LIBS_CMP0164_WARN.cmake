enable_language(CXX)
set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)
add_library(someLib SHARED test.cpp)
