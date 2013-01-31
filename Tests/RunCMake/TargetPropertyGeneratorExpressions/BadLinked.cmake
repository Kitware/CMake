
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/main.cpp"
           "int main(int, char **) { return 0; }\n")

add_executable(TargetPropertyGeneratorExpressions
           "${CMAKE_CURRENT_BINARY_DIR}/main.cpp")
target_link_libraries(TargetPropertyGeneratorExpressions "$<LINKED:something>")
