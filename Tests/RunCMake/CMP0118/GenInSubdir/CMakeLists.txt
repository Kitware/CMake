add_custom_command(
  OUTPUT sub.txt
  COMMAND ${CMAKE_COMMAND} -E touch sub.txt
  VERBATIM
  )
add_custom_target(gen DEPENDS sub.txt)

add_dependencies(top gen)
target_sources(top PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/sub.txt)
