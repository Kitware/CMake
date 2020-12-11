include(${CMAKE_CURRENT_LIST_DIR}/CMP0118-Common-Helper.cmake)


add_custom_target(custom1)
target_sources(custom1 PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/Generated_with_full_path1.txt"
)
add_custom_target(custom2)
target_sources(custom2 PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/Generated_with_full_path2.txt"
)
add_custom_target(custom3)
target_sources(custom3 PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/Generated_with_full_path3.txt"
)
add_custom_target(custom4)
target_sources(custom4 PRIVATE
  "Generated_with_relative_path1.txt"
)
add_custom_target(custom5)
target_sources(custom5 PRIVATE
  "Generated_with_relative_path2.txt"
)
add_custom_target(custom6)
target_sources(custom6 PRIVATE
  "Generated_with_relative_path3.txt"
)
add_custom_target(custom7)
target_sources(custom7 PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Generated_with_full_source_path1.txt"
)
add_custom_target(custom8)
target_sources(custom8 PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Generated_with_full_source_path2.txt"
)
add_custom_target(custom9)
target_sources(custom9 PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Generated_with_full_source_path3.txt"
)


set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/Generated_with_full_path1.txt"
  PROPERTY GENERATED "tRue")
get_and_print_GENERATED_property("Generated_with_full_path1.txt")

set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/Generated_with_full_path2.txt"
  PROPERTY GENERATED "SomeVar-NOTFOUND")
get_and_print_GENERATED_property("Generated_with_full_path2.txt")

set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/Generated_with_full_path3.txt"
  PROPERTY GENERATED "Junk-value")
get_and_print_GENERATED_property("Generated_with_full_path3.txt")


set_property(SOURCE "Generated_with_relative_path1.txt"
  PROPERTY GENERATED "tRue")
get_and_print_GENERATED_property("Generated_with_relative_path1.txt")

set_property(SOURCE "Generated_with_relative_path2.txt"
  PROPERTY GENERATED "SomeVar-NOTFOUND")
get_and_print_GENERATED_property("Generated_with_relative_path2.txt")

set_property(SOURCE "Generated_with_relative_path3.txt"
  PROPERTY GENERATED "Junk-value")
get_and_print_GENERATED_property("Generated_with_relative_path3.txt")


set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/Generated_with_full_source_path1.txt"
  PROPERTY GENERATED "tRue")
get_and_print_GENERATED_property("Generated_with_full_source_path1.txt")

set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/Generated_with_full_source_path2.txt"
  PROPERTY GENERATED "SomeVar-NOTFOUND")
get_and_print_GENERATED_property("Generated_with_full_source_path2.txt")

set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/"Generated_with_full_source_path3.txt"
  PROPERTY GENERATED "Junk-value")
get_and_print_GENERATED_property("Generated_with_full_source_path3.txt")
