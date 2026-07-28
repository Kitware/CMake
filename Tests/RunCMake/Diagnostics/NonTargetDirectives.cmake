cmake_minimum_required(VERSION 4.4)
project(NonTargetDirectives)

# Test diagnostics for non-target directives. Note that, because we don't
# need to actually compile anything, it doesn't matter if we use options,
# directories, or libraries that don't exist.

add_definitions(-DFOO)

add_compile_definitions(BAR)
add_compile_options(-fPIC)

add_link_options(-Wl,--no-undefined)

include_directories(${CMAKE_SOURCE_DIR}/include)

link_directories(${CMAKE_SOURCE_DIR}/lib)

link_libraries(/usr/lib/libm.so)
