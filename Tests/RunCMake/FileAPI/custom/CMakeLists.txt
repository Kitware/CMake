project(Custom)

add_custom_target(custom_tgt COMMAND ${CMAKE_COMMAND} -E echo "Building custom_tgt")
add_executable(custom_exe ../empty.c)
add_dependencies(custom_exe custom_tgt)
