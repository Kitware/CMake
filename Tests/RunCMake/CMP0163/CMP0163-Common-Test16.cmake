include(${CMAKE_CURRENT_LIST_DIR}/CMP0163-Common-Helper.cmake)


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


add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/Generated_source1.txt"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Source.txt.in"
                                   "${CMAKE_CURRENT_BINARY_DIR}/Generated_source1.txt"
)
add_custom_command(TARGET custom2 PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Source.txt.in"
                                   "${CMAKE_CURRENT_BINARY_DIR}/Generated_source2.txt"
  BYPRODUCT "${CMAKE_CURRENT_BINARY_DIR}/Generated_source2.txt"
)

add_subdirectory(subdir-Common-Test16)

get_and_print_GENERATED_property("Generated_source1.txt")
get_and_print_GENERATED_property("Generated_source2.txt")
get_and_print_GENERATED_property("Generated_source3.txt")
get_and_print_GENERATED_property("Generated_source4.txt")
