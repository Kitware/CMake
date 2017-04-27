include(Compiler/XL)
__compiler_xl(CXX)
string(APPEND CMAKE_CXX_FLAGS_RELEASE_INIT " -DNDEBUG")
string(APPEND CMAKE_CXX_FLAGS_MINSIZEREL_INIT " -DNDEBUG")

# -qthreaded     = Ensures that all optimizations will be thread-safe
string(APPEND CMAKE_CXX_FLAGS_INIT " -qthreaded")

set(CMAKE_CXX_COMPILE_OBJECT
  "<CMAKE_CXX_COMPILER> -+ <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> -c <SOURCE>")
