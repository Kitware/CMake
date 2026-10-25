file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/input.txt "1")

add_custom_command(
  OUTPUT generated.h
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/input.txt
                                   ${CMAKE_CURRENT_BINARY_DIR}/generated.h
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/input.txt
  CODEGEN
)

add_custom_target(h_creator ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/generated.h)
