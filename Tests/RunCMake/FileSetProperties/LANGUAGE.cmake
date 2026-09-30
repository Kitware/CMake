add_library(foo STATIC)
target_sources(foo PRIVATE FILE_SET sources TYPE SOURCES FILES foo.c)
set_property(FILE_SET sources TARGET foo PROPERTY LANGUAGE CXX)

get_property(language FILE_SET sources TARGET foo PROPERTY LANGUAGE)
if(NOT language STREQUAL "CXX")
  message(SEND_ERROR "wrong language: '${language}' instead of 'CXX'")
endif()
