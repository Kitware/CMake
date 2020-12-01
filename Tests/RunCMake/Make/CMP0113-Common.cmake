add_custom_command(
  OUTPUT output-not-created
  COMMAND ${CMAKE_COMMAND} -E echo output-not-created
  DEPENDS ${CMAKE_CURRENT_LIST_FILE}
  VERBATIM
  )

add_custom_command(
  OUTPUT output-created
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_FILE} output-created
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/output-not-created
  VERBATIM
  )

add_custom_target(drive1 ALL DEPENDS output-not-created)
add_custom_target(drive2 ALL DEPENDS output-created)
add_dependencies(drive2 drive1)
