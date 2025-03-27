add_custom_target(foobar
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/generated.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated.h
)

add_custom_command(TARGET foobar POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E true
  CODEGEN
)
