add_library(TargetImported UNKNOWN IMPORTED)
add_custom_command(TARGET TargetImported POST_BUILD COMMAND ${CMAKE_COMMAND} -E echo tada)
