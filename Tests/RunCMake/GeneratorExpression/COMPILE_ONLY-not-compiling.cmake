add_custom_target(Custom ALL COMMAND ${CMAKE_COMMAND} -E echo $<COMPILE_ONLY:something>)
