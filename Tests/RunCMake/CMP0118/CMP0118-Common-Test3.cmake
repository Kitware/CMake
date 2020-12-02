include(${CMAKE_CURRENT_LIST_DIR}/CMP0118-Common-Helper.cmake)


add_executable(executable1)
target_sources(executable1 PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/Generated_with_full_path1.cpp"
  "${CMAKE_CURRENT_BINARY_DIR}/Generated_with_full_path2.cpp"
  "${CMAKE_CURRENT_BINARY_DIR}/Generated_with_full_path3.cpp"
)
add_executable(executable2)
target_sources(executable2 PRIVATE
  "Generated_with_relative_path1.cpp"
  "Generated_with_relative_path2.cpp"
  "Generated_with_relative_path3.cpp"
)
add_executable(executable3)
target_sources(executable3 PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Generated_with_full_source_path1.cpp"
)
add_executable(executable4)
target_sources(executable4 PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Generated_with_full_source_path2.cpp"
)
add_executable(executable5)
target_sources(executable5 PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Generated_with_full_source_path3.cpp"
)


set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/Generated_with_full_path1.cpp"
  PROPERTY GENERATED "1")
get_and_print_GENERATED_property("Generated_with_full_path1.cpp")

set_property(SOURCE "Generated_with_full_path2.cpp"
  PROPERTY GENERATED "1")
get_and_print_GENERATED_property("Generated_with_full_path2.cpp")

set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/"Generated_with_full_path3.cpp"
  PROPERTY GENERATED "1")
get_and_print_GENERATED_property("Generated_with_full_path3.cpp")


set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/Generated_with_relative_path1.cpp"
  PROPERTY GENERATED "1")
get_and_print_GENERATED_property("Generated_with_relative_path1.cpp")

set_property(SOURCE "Generated_with_relative_path2.cpp"
  PROPERTY GENERATED "1")
get_and_print_GENERATED_property("Generated_with_relative_path2.cpp")

set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/Generated_with_relative_path3.cpp"
  PROPERTY GENERATED "1")
get_and_print_GENERATED_property("Generated_with_relative_path3.cpp")


set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/Generated_with_full_source_path1.cpp"
  PROPERTY GENERATED "1")
get_and_print_GENERATED_property("Generated_with_full_source_path1.cpp")

set_property(SOURCE "Generated_with_full_source_path2.cpp"
  PROPERTY GENERATED "1")
get_and_print_GENERATED_property("Generated_with_full_source_path2.cpp")

set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/"Generated_with_full_source_path3.cpp"
  PROPERTY GENERATED "1")
get_and_print_GENERATED_property("Generated_with_full_source_path3.cpp")
