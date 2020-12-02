include(${CMAKE_CURRENT_LIST_DIR}/CMP0118-Common-Helper.cmake)


add_executable(executable1)
target_sources(executable1 PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/Generated_source1.cpp"
)
add_executable(executable2)
target_sources(executable2 PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/Generated_source2.cpp"
)
add_executable(executable3)
target_sources(executable3 PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/Generated_source3.cpp"
)
add_executable(executable4)
target_sources(executable4 PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/Generated_source4.cpp"
)
add_executable(executable5)
target_sources(executable5 PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/Generated_source5.cpp"
)
add_executable(executable6)
target_sources(executable6 PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/Generated_source6.cpp"
)


set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/Generated_source1.cpp"
  PROPERTY GENERATED "1")
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/Generated_source2.cpp"
  PROPERTY GENERATED "1")
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/Generated_source3.cpp"
  PROPERTY GENERATED "1")

add_subdirectory(subdir-Common-Test6)

get_and_print_GENERATED_property("Generated_source1.cpp")
get_and_print_GENERATED_property("Generated_source2.cpp")
get_and_print_GENERATED_property("Generated_source3.cpp")
get_and_print_GENERATED_property("Generated_source4.cpp")
get_and_print_GENERATED_property("Generated_source5.cpp")
get_and_print_GENERATED_property("Generated_source6.cpp")
