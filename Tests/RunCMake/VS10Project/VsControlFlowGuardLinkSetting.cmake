enable_language(CXX)

# Add the Control Flow Guard compiler and linker option
add_compile_options("/guard:cf")
string(APPEND CMAKE_SHARED_LINKER_FLAGS " /guard:cf")

add_library(ControlFlowGuardProject SHARED foo.cpp)
