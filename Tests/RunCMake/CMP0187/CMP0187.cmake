add_custom_command(OUTPUT z.h COMMAND ${CMAKE_COMMAND} -E true)
add_custom_command(OUTPUT z COMMAND ${CMAKE_COMMAND} -E false)

add_library(lib INTERFACE z z.h)
