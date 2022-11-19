message(STATUS "Testing the configure step at ${CMAKE_BINARY_DIR}")

add_custom_target(echo_test ALL COMMAND ${CMAKE_COMMAND} -E echo "Testing the build step at ${CMAKE_BINARY_DIR}")

enable_testing()
add_test(NAME EchoTest COMMAND ${CMAKE_COMMAND} -E echo "Testing the test step at ${CMAKE_BINARY_DIR}")

include(CPack)
