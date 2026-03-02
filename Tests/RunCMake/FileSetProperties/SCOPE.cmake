
enable_language(C)

add_library(foo STATIC foo.c)
target_sources(foo PUBLIC FILE_SET HEADERS FILES foo.h)

get_property(scope FILE_SET HEADERS TARGET foo PROPERTY SCOPE)
if(NOT scope STREQUAL "PUBLIC")
  message(SEND_ERROR "wrong scope: ${scope} instead of PUBLIC")
endif()
