# Prevent duplicate errors on some platforms.
set(CMAKE_IMPORT_LIBRARY_SUFFIX "placeholder")

add_library(unknown_lib UNKNOWN IMPORTED)
add_library(static_lib STATIC IMPORTED)
add_library(shared_lib SHARED IMPORTED)

add_executable(executable main.cpp)
target_link_libraries(executable unknown_lib static_lib shared_lib)
