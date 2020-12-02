add_custom_command(TARGET executable4 PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Source.cpp.in"
                                   "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source4.cpp"
  BYPRODUCT "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source4.cpp"
)
add_custom_command(TARGET executable5 PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Source.cpp.in"
                                   "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source5.cpp"
  BYPRODUCT "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source5.cpp"
)
add_custom_command(TARGET executable6 PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Source.cpp.in"
                                   "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source6.cpp"
  BYPRODUCT "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source6.cpp"
)


set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source1.cpp"
  PROPERTY GENERATED "0")
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source2.cpp"
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
  PROPERTY GENERATED "0")
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source3.cpp"
  TARGET_DIRECTORY executable1
  PROPERTY GENERATED "0")
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source4.cpp"
  PROPERTY GENERATED "0")
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source5.cpp"
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
  PROPERTY GENERATED "0")
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source6.cpp"
  TARGET_DIRECTORY executable1
  PROPERTY GENERATED "0")
