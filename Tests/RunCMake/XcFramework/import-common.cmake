enable_language(C)

find_package(mylib REQUIRED)

add_custom_target(print_loc ALL COMMAND ${CMAKE_COMMAND} -E echo "mylib location: $<TARGET_FILE:mylib>")
