enable_language(C)

add_library(Unset STATIC empty.c)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_library(SetOn STATIC empty.c)
set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)
add_library(SetOff STATIC empty.c)

get_property(_set TARGET Unset PROPERTY EXPORT_COMPILE_COMMANDS)
if(_set)
  message(SEND_ERROR "EXPORT_COMPILE_COMMANDS property should be unset for Unset target (got \"${_set}\")")
endif()

get_property(_on TARGET SetOn PROPERTY EXPORT_COMPILE_COMMANDS)
if(NOT _on STREQUAL "ON")
  message(SEND_ERROR "EXPORT_COMPILE_COMMANDS property should be \"ON\" for SetOn target (got \"${_on}\")")
endif()

get_property(_off TARGET SetOff PROPERTY EXPORT_COMPILE_COMMANDS)
if(NOT _off STREQUAL "OFF")
  message(SEND_ERROR "EXPORT_COMPILE_COMMANDS property should be \"OFF\" for SetOff target (got \"${_off}\")")
endif()
