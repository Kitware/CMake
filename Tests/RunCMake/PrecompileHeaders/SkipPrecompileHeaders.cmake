enable_language(C)
enable_language(CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(pch-test main.cpp non-pch.cpp)
target_precompile_headers(pch-test PRIVATE pch.h)

set_source_files_properties(non-pch.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

enable_testing()
add_test(NAME pch-test COMMAND pch-test)
