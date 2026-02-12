# Test that isolated files from subdirectories get proper subdirectory-based object paths,
# while non-isolated files are combined in unity buckets regardless of subdirectory.

set(CMAKE_UNITY_BUILD ON)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/subdir1)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/subdir2)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/main.cpp "int main() { return 0; }\n")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/subdir1/unity_file.cpp "int unity_file() { return 1; }\n")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/subdir2/isolated_file.cpp "int isolated_file() { return 2; }\n")

add_executable(main
    ${CMAKE_CURRENT_BINARY_DIR}/main.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/subdir1/unity_file.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/subdir2/isolated_file.cpp
)

set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/subdir2/isolated_file.cpp
    TARGET_DIRECTORY main
    PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON
)
