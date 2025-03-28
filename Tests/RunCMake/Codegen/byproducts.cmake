add_custom_target(foobar
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/generated.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated.h
  BYPRODUCTS
    ${CMAKE_CURRENT_BINARY_DIR}/generated.h
)

# This codegen step relies on the BYPRODUCTS of the previous command.
# If foobar isn't properly accounted for as a dependency it will fail.
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated.hpp
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/generated.h
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/generated.h
                                   ${CMAKE_CURRENT_BINARY_DIR}/generated.hpp
  CODEGEN
)

add_custom_target(hpp_creator ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/generated.hpp)
