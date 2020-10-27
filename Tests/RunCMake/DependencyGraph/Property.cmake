enable_language(C)

add_library(Unset STATIC mylib.c)

set(CMAKE_OPTIMIZE_DEPENDENCIES TRUE)
add_library(SetTrue STATIC mylib.c)

set(CMAKE_OPTIMIZE_DEPENDENCIES FALSE)
add_library(SetFalse STATIC mylib.c)

get_property(_set TARGET Unset PROPERTY OPTIMIZE_DEPENDENCIES SET)
if(_set)
  message(SEND_ERROR "OPTIMIZE_DEPENDENCIES property should not be set on Unset target")
endif()

get_property(_true TARGET SetTrue PROPERTY OPTIMIZE_DEPENDENCIES)
if(NOT _true STREQUAL "TRUE")
  message(SEND_ERROR "OPTIMIZE_DEPENDENCIES property should be TRUE on SetTrue target")
endif()

get_property(_false TARGET SetFalse PROPERTY OPTIMIZE_DEPENDENCIES)
if(NOT _false STREQUAL "FALSE")
  message(SEND_ERROR "OPTIMIZE_DEPENDENCIES property should be FALSE on SetFalse target")
endif()
