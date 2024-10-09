enable_language(CXX)
cmake_policy(SET CMP0164 NEW)
set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)
add_library(someLib SHARED test.cpp)
