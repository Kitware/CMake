get_property(role GLOBAL PROPERTY CMAKE_ROLE)
if(NOT role STREQUAL "PROJECT")
  message(SEND_ERROR "CMAKE_ROLE property is \"${role}\", should be \"PROJECT\"")
endif()
