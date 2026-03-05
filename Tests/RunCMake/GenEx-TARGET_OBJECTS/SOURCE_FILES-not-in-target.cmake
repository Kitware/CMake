enable_language(C)

add_library(c_objs OBJECT empty1.c empty2.c)
add_library(other OBJECT empty3.c)

add_custom_target(obj ALL
  COMMAND ${CMAKE_COMMAND} -E echo $<TARGET_OBJECTS:c_objs,SOURCE_FILES:${CMAKE_CURRENT_SOURCE_DIR}/empty3.c>
  VERBATIM
  )
