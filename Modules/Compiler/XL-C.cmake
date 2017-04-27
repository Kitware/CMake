include(Compiler/XL)
__compiler_xl(C)
string(APPEND CMAKE_C_FLAGS_RELEASE_INIT " -DNDEBUG")
string(APPEND CMAKE_C_FLAGS_MINSIZEREL_INIT " -DNDEBUG")

# -qthreaded     = Ensures that all optimizations will be thread-safe
string(APPEND CMAKE_C_FLAGS_INIT " -qthreaded")
