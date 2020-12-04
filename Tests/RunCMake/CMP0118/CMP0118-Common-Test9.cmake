include(${CMAKE_CURRENT_LIST_DIR}/CMP0118-Common-Helper.cmake)


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


file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/Generated_source1.txt"
  CONTENT "int func();\nint main(){ return func(); }"
)
file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/Generated_source2.txt"
  CONTENT "int func();\nint main(){ return func(); }"
)
file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/Generated_source3.txt"
  CONTENT "int func();\nint main(){ return func(); }"
)

add_subdirectory(subdir-Common-Test9)

get_and_print_GENERATED_property("Generated_source1.txt")
get_and_print_GENERATED_property("Generated_source2.txt")
get_and_print_GENERATED_property("Generated_source3.txt")
get_and_print_GENERATED_property("Generated_source4.txt")
get_and_print_GENERATED_property("Generated_source5.txt")
get_and_print_GENERATED_property("Generated_source6.txt")
