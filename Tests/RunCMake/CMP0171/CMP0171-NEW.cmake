# codegen is now a reserved name and this will cause an error since the policy is new.
add_custom_target(codegen
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/generated.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated.h
)
