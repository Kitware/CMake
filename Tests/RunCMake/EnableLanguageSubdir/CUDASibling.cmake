
# With CMP0220 NEW, CUDA enabled in a subdirectory propagates up to this scope.
cmake_policy(SET CMP0220 NEW)

enable_language(CXX)

add_subdirectory(cuda)

add_subdirectory(sibling)
