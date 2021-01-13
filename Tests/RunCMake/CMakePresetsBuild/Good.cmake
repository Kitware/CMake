add_custom_target(good ALL)
add_custom_command(TARGET good PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E environment)
