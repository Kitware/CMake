enable_language(C)

add_library(c_objs IMPORTED OBJECT)

add_custom_target(obj ALL
  COMMAND ${CMAKE_COMMAND} -E echo $<TARGET_OBJECTS:c_objs,SOURCE_FILES:${CMAKE_CURRENT_SOURCE_DIR}/empty4.c>
  VERBATIM
  )
