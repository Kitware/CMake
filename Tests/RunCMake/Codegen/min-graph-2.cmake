add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/generated.h
  COMMAND
    ${CMAKE_COMMAND} -E
        copy ${CMAKE_CURRENT_SOURCE_DIR}/generated.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/generated.h
  CODEGEN
)

# This target should not be built. It has no reason
# to be part of the codegen build graph
add_custom_target(error_custom_target ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/generated.h

  # Cause the build to fail
  COMMAND ${CMAKE_COMMAND} -E false
)
