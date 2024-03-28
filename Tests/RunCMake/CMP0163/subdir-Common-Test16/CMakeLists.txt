add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source3.txt"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Source.txt.in"
                                   "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source3.txt"
)
add_custom_command(TARGET custom4 PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Source.txt.in"
                                   "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source4.txt"
  BYPRODUCT "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source4.txt"
)
