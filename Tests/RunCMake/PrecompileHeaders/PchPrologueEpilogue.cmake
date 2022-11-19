enable_language(C)
enable_language(CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_PCH_PROLOGUE "#pragma warning(push, 0)")
set(CMAKE_PCH_EPILOGUE "#pragma warning(pop)")

add_executable(main main.cpp)
target_precompile_headers(main PRIVATE pch.h)
