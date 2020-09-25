include(${CMAKE_CURRENT_LIST_DIR}/CMP0118-Common-Helper.cmake)


# The sources of executable0 will not be modified by set_property!
add_executable(executable0)
target_sources(executable0 PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/Generated_source0.cpp"
)
# The sources of executable[1-6] will (tried to) be modified by set_property!
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


add_custom_command(TARGET executable0 PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Source.cpp.in"
                                   "${CMAKE_CURRENT_BINARY_DIR}/Generated_source0.cpp"
  BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/Generated_source0.cpp"
)
add_custom_command(TARGET executable1 PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Source.cpp.in"
                                   "${CMAKE_CURRENT_BINARY_DIR}/Generated_source1.cpp"
  BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/Generated_source1.cpp"
)
add_custom_command(TARGET executable2 PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Source.cpp.in"
                                   "${CMAKE_CURRENT_BINARY_DIR}/Generated_source2.cpp"
  BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/Generated_source2.cpp"
)
add_custom_command(TARGET executable3 PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Source.cpp.in"
                                   "${CMAKE_CURRENT_BINARY_DIR}/Generated_source3.cpp"
  BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/Generated_source3.cpp"
)

add_subdirectory(subdir-Common-Test13)

get_and_print_GENERATED_property("Generated_source0.cpp")
get_and_print_GENERATED_property("Generated_source1.cpp")
get_and_print_GENERATED_property("Generated_source2.cpp")
get_and_print_GENERATED_property("Generated_source3.cpp")
get_and_print_GENERATED_property("Generated_source4.cpp")
get_and_print_GENERATED_property("Generated_source5.cpp")
get_and_print_GENERATED_property("Generated_source6.cpp")
