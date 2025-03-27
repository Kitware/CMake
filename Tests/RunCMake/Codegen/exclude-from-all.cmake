add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/generated.h
  COMMAND
    ${CMAKE_COMMAND} -E false
  CODEGEN
)

# We don't want codegen to drive parts of the project that are EXCLUDE_FROM_ALL.
# This tests that foobar is properly excluded from the codegen build.
add_executable(foobar EXCLUDE_FROM_ALL error.c ${CMAKE_CURRENT_BINARY_DIR}/generated.h)
