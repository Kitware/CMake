add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated.hpp
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/generated.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated.hpp
  # This will cause an error since the CODEGEN option
  # requires that CMP0171 is set to NEW
  CODEGEN
)
