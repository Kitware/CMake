enable_language(C)

add_library(c_objs OBJECT empty1.c empty2.c empty3.c)

add_custom_target(obj ALL
  COMMAND ${CMAKE_COMMAND} -E echo $<TARGET_OBJECTS:c_objs,INVALID_ARGUMENT_1,INVALID_ARGUMENT_2>
  VERBATIM
  )
