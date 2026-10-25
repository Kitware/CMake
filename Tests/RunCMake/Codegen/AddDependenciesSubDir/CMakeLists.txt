add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/generated.h
  COMMAND
    ${CMAKE_COMMAND} -E
        copy ${CMAKE_CURRENT_SOURCE_DIR}/../generated.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/generated.h
  CODEGEN
)
add_custom_target(h_creator DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/generated.h)

# Nothing depends on this, and the directory is EXCLUDE_FROM_ALL,
# so codegen must not run it.
add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/unreached.h
  COMMAND
    ${CMAKE_COMMAND} -E false
  CODEGEN
)
add_custom_target(unreached DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/unreached.h)
