cmake_minimum_required(VERSION 3.14)
project(env-export-compile-commands C)

# Add target with a source file to make sure compile_commands.json gets
# generated.
file(TOUCH ${CMAKE_CURRENT_BINARY_DIR}/main.c)
add_executable(env-export-compile-commands ${CMAKE_CURRENT_BINARY_DIR}/main.c)
