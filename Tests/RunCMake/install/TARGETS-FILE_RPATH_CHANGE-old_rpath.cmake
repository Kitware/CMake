enable_language(C)

add_library(utils SHARED obj1.c)

# exe1: absolute build RPATH, no cmake syntax
set(CMAKE_BUILD_RPATH_USE_ORIGIN OFF)
set(CMAKE_INSTALL_RPATH "/foo/bar")
add_executable(exe1 main.c)
target_link_libraries(exe1 PRIVATE utils)

# exe2: relative build RPATH, no cmake syntax
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
set(CMAKE_INSTALL_RPATH "/foo/bar")
add_executable(exe2 main.c)
target_link_libraries(exe2 PRIVATE utils)

install(TARGETS utils exe1 exe2)
