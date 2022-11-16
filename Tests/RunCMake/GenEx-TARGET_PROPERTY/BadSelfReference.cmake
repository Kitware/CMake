enable_language(CXX)
add_subdirectory(BadSelfReference1)
add_subdirectory(BadSelfReference2)
add_subdirectory(BadSelfReference3)
add_subdirectory(BadSelfReference4)
add_subdirectory(BadSelfReference5)
add_subdirectory(BadSelfReference6)

# Suppress generator-specific targets that might pollute the stderr.
set(CMAKE_SUPPRESS_REGENERATION TRUE)
