
cmake_minimum_required(VERSION 2.8)

project(CompileDefinitions)

add_subdirectory(add_definitions_command)
add_subdirectory(target_prop)
add_subdirectory(add_definitions_command_with_target_prop)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/main.cpp" "int main(int, char **) { return 0; }\n")

add_executable(CompileDefinitions "${CMAKE_CURRENT_BINARY_DIR}/main.cpp")
