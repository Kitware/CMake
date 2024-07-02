# CMake should warn the user if they have a target named codegen.
add_custom_target(codegen
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/generated.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated.h
)
