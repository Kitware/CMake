
enable_language(C)

add_library(foo STATIC foo.c)
target_sources(foo PUBLIC FILE_SET HEADERS FILES foo.h)

get_property(type FILE_SET HEADERS TARGET foo PROPERTY TYPE)
if(NOT type STREQUAL "HEADERS")
  message(SEND_ERROR "wrong type: ${type} instead of HEADERS")
endif()
