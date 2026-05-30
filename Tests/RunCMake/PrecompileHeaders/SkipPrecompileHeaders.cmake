set(CMAKE_INTERMEDIATE_DIR_STRATEGY FULL CACHE STRING "" FORCE)

enable_language(C)
enable_language(CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(pch-test main.cpp non-pch1.cpp)
target_precompile_headers(pch-test PRIVATE pch.h)

set_source_files_properties(non-pch1.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

target_sources(pch-test PRIVATE FILE_SET f1 TYPE SOURCES FILES non-pch2.cpp)
set_property(FILE_SET f1 TARGET pch-test PROPERTY SKIP_PRECOMPILE_HEADERS ON)

target_sources(pch-test PRIVATE FILE_SET f2 TYPE SOURCES FILES non-pch3.cpp)

set_source_files_properties(non-pch3.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

enable_testing()
add_test(NAME pch-test COMMAND pch-test)
