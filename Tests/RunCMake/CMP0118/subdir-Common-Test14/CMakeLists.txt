add_custom_target(executable4_source_generator ALL
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Source.cpp.in"
                                   "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source4.cpp"
  BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source4.cpp"
)
add_custom_target(executable5_source_generator ALL
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Source.cpp.in"
                                   "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source5.cpp"
  BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source5.cpp"
)
add_custom_target(executable6_source_generator ALL
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Source.cpp.in"
                                   "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source6.cpp"
  BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source6.cpp"
)


set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source1.cpp"
  PROPERTY GENERATED "1")
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source2.cpp"
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
  PROPERTY GENERATED "1")
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source3.cpp"
  TARGET_DIRECTORY executable1
  PROPERTY GENERATED "1")
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source4.cpp"
  PROPERTY GENERATED "1")
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source5.cpp"
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
  PROPERTY GENERATED "1")
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/../Generated_source6.cpp"
  TARGET_DIRECTORY executable1
  PROPERTY GENERATED "1")
