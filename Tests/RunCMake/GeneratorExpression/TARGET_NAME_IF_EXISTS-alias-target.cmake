enable_language(CXX)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/dummy_executable.cpp" "int main(int,char**) { return 0; }\n")
add_executable(executableTarget "${CMAKE_CURRENT_BINARY_DIR}/dummy_executable.cpp")
add_executable(aliasTarget ALIAS executableTarget)

cmake_policy(SET CMP0070 NEW)
file(GENERATE OUTPUT TARGET_NAME_IF_EXISTS-generated-alias.txt CONTENT "$<TARGET_NAME_IF_EXISTS:aliasTarget>")
