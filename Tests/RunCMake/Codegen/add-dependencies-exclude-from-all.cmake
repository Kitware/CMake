add_custom_command(
  OUTPUT generated.h
  COMMAND ${CMAKE_COMMAND} -E false
  CODEGEN
)
add_custom_target(h_creator DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/generated.h)

# codegen should not follow dependencies of targets that are EXCLUDE_FROM_ALL.
add_executable(main EXCLUDE_FROM_ALL main.c)
add_dependencies(main h_creator)
