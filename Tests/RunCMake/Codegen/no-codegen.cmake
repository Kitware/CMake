add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated.hpp
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/generated.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated.hpp
)
