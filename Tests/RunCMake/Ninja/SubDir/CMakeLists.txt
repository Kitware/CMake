add_custom_target(SubFail COMMAND does_not_exist)
add_custom_target(InAll ALL COMMAND ${CMAKE_COMMAND} -E echo "Building InAll")
