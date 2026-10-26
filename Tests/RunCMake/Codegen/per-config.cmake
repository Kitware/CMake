add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/generated.h
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/generated.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/generated.h
  CODEGEN
)

add_custom_target(h_creator ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/generated.h
)
