add_subdirectory(TargetNotInDir)
add_custom_command(TARGET TargetNotInDir POST_BUILD COMMAND ${CMAKE_COMMAND} -E echo tada)
