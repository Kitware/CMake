# Note: Target "custom4" was not created in this directory, so setting this build-event will fail!
add_custom_command(TARGET custom4 PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Source.txt.in"
                                   "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source4.txt"
  BYPRODUCT "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source4.txt"
)
# Note: Target "custom5" was not created in this directory, so setting this build-event will fail!
add_custom_command(TARGET custom5 PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Source.txt.in"
                                   "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source5.txt"
  BYPRODUCT "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source5.txt"
)
# Note: Target "custom6" was not created in this directory, so setting this build-event will fail!
add_custom_command(TARGET custom6 PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Source.txt.in"
                                   "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source6.txt"
  BYPRODUCT "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source6.txt"
)


set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source1.txt"
  PROPERTY GENERATED "0")
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source2.txt"
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
  PROPERTY GENERATED "0")
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source3.txt"
  TARGET_DIRECTORY custom1
  PROPERTY GENERATED "0")
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source4.txt"
  PROPERTY GENERATED "0")
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source5.txt"
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
  PROPERTY GENERATED "0")
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source6.txt"
  TARGET_DIRECTORY custom1
  PROPERTY GENERATED "0")
