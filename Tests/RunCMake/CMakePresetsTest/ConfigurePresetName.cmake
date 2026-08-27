enable_testing()
add_test(NAME print-environment COMMAND ${CMAKE_COMMAND} -E environment)
