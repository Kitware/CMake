enable_language(CXX)
add_subdirectory(BadInvalidName1)
add_subdirectory(BadInvalidName2)
add_subdirectory(BadInvalidName3)
add_subdirectory(BadInvalidName4)
add_subdirectory(BadInvalidName5)
add_subdirectory(BadInvalidName6)
add_subdirectory(BadInvalidName7)
add_subdirectory(BadInvalidName8)

# Suppress generator-specific targets that might pollute the stderr.
set(CMAKE_SUPPRESS_REGENERATION TRUE)
