add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/main.cpp
  COMMAND
    ${CMAKE_COMMAND} -E
        copy ${CMAKE_CURRENT_SOURCE_DIR}/error.c
        ${CMAKE_CURRENT_BINARY_DIR}/main.cpp
  CODEGEN
  # ERROR out if IMPLICIT_DEPENDS is used with CODEGEN
  IMPLICIT_DEPENDS C main.c
)
