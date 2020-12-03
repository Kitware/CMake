include(${CMAKE_CURRENT_LIST_DIR}/CMP0118-Common-Helper.cmake)


# The sources of custom0 will not be modified by set_property!
add_custom_target(custom0)
target_sources(custom0 PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/Generated_source0.txt"
)
# The sources of custom[1-6] will (tried to) be modified by set_property!
add_custom_target(custom1)
target_sources(custom1 PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/Generated_source1.txt"
)
add_custom_target(custom2)
target_sources(custom2 PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/Generated_source2.txt"
)
add_custom_target(custom3)
target_sources(custom3 PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/Generated_source3.txt"
)
add_custom_target(custom4)
target_sources(custom4 PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/Generated_source4.txt"
)
add_custom_target(custom5)
target_sources(custom5 PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/Generated_source5.txt"
)
add_custom_target(custom6)
target_sources(custom6 PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/Generated_source6.txt"
)


add_custom_target(custom0_source_generator ALL
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Source.txt.in"
                                   "${CMAKE_CURRENT_BINARY_DIR}/Generated_source0.txt"
  BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/Generated_source0.txt"
)
add_custom_target(custom1_source_generator ALL
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Source.txt.in"
                                   "${CMAKE_CURRENT_BINARY_DIR}/Generated_source1.txt"
  BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/Generated_source1.txt"
)
add_custom_target(custom2_source_generator ALL
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Source.txt.in"
                                   "${CMAKE_CURRENT_BINARY_DIR}/Generated_source2.txt"
  BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/Generated_source2.txt"
)
add_custom_target(custom3_source_generator ALL
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Source.txt.in"
                                   "${CMAKE_CURRENT_BINARY_DIR}/Generated_source3.txt"
  BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/Generated_source3.txt"
)

add_subdirectory(subdir-Common-Test15)

get_and_print_GENERATED_property("Generated_source0.txt")
get_and_print_GENERATED_property("Generated_source1.txt")
get_and_print_GENERATED_property("Generated_source2.txt")
get_and_print_GENERATED_property("Generated_source3.txt")
get_and_print_GENERATED_property("Generated_source4.txt")
get_and_print_GENERATED_property("Generated_source5.txt")
get_and_print_GENERATED_property("Generated_source6.txt")
