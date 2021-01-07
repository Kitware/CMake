enable_language(C)

add_executable(Exe main.c)
add_custom_command(TARGET Exe POST_BUILD COMMAND ${CMAKE_COMMAND} -E echo "Running post-build command with $<CONFIG>")
add_custom_command(TARGET Exe POST_BUILD COMMAND ${CMAKE_COMMAND} -E echo "Generating out.txt with $<CONFIG>" BYPRODUCTS out.txt)
add_custom_command(TARGET Exe POST_BUILD COMMAND ${CMAKE_COMMAND} -E echo "Generating $<CONFIG>.txt" BYPRODUCTS $<CONFIG>.txt)
