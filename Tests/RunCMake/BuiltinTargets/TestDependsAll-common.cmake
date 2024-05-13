enable_testing()
add_custom_target(custom ALL COMMAND ${CMAKE_COMMAND} -E touch custom-output.txt)
add_test(NAME test COMMAND ${CMAKE_COMMAND} -E echo)
