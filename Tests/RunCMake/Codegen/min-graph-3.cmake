add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/error_lib.c
  COMMAND
    ${CMAKE_COMMAND} -E
        copy ${CMAKE_CURRENT_SOURCE_DIR}/error.c
        ${CMAKE_CURRENT_BINARY_DIR}/error_lib.c
  CODEGEN
)

# This test will fail if error_lib.c is actually compiled
add_executable(foobar ${CMAKE_CURRENT_BINARY_DIR}/error_lib.c)
