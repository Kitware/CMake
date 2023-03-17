enable_language(CXX)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/main.cpp"
           "int main(int, char **) { return 0; }\n")

add_executable(main "${CMAKE_CURRENT_BINARY_DIR}/main.cpp")
include_directories("$<TARGET_PROPERTY:NonExistent,INCLUDE_DIRECTORIES>")

# Suppress generator-specific targets that might pollute the stderr.
set(CMAKE_SUPPRESS_REGENERATION TRUE)
