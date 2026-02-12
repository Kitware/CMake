# Test that unity build combines files from different subdirectories into the same unity bucket.

set(CMAKE_UNITY_BUILD ON)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/subdir1)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/subdir2)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/main.cpp "int main() { return 0; }\n")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/subdir1/file1.cpp "int file1() { return 1; }\n")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/subdir2/file2.cpp "int file2() { return 2; }\n")

add_executable(main
    ${CMAKE_CURRENT_BINARY_DIR}/main.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/subdir1/file1.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/subdir2/file2.cpp
)
