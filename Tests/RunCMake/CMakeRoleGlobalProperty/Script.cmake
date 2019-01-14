cmake_minimum_required(VERSION 3.12)

get_property(role GLOBAL PROPERTY CMAKE_ROLE)
if(NOT role STREQUAL "SCRIPT")
  message(SEND_ERROR "CMAKE_ROLE property is \"${role}\", should be \"SCRIPT\"")
endif()
