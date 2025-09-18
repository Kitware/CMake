# Generic
set(CMAKE_FASTBUILD_USE_RELATIVE_PATHS ON)
set(CMAKE_FASTBUILD_USE_DETERMINISTIC_PATHS ON)
set(CMAKE_FASTBUILD_SOURCE_MAPPING "/new/root")
set(CMAKE_FASTBUILD_ALLOW_RESPONSE_FILE ON)
# This should not appear in the generated file, since it's a default value.
set(CMAKE_FASTBUILD_FORCE_RESPONSE_FILE OFF)
set(CMAKE_FASTBUILD_COMPILER_EXTRA_FILES "file1;file2")

add_executable(main main.cpp)
