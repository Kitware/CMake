enable_language(CXX)
cmake_policy(SET CMP0164 OLD)
set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)

add_library(someLib SHARED test.cpp)
get_target_property(someLibType someLib TYPE)
if(NOT someLibType STREQUAL  "STATIC_LIBRARY")
  message(FATAL_ERROR "expected type does not match")
endif()
