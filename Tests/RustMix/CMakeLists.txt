cmake_minimum_required(VERSION 4.2)

set(CMAKE_EXPERIMENTAL_RUST "6b6e613b-f6cb-402d-8aea-59034fa8c65b")

project(RustMix LANGUAGES C CXX Rust)

add_library(rs_staticlib STATIC rs_staticlib.rs)
add_library(rs_cdylib    SHARED rs_cdylib.rs)
add_library(rs_rlib      OBJECT rs_rlib.rs)

add_library(c_static STATIC c_static.c)
add_library(c_shared SHARED c_shared.c)
add_library(c_obj    OBJECT c_obj.c)

add_library(cpp_shared SHARED cpp_shared.cpp)
# Note: trying to link a C++ object file or static library into the Rust
# executable currently fails with errors related to missing symbols from
# libstdc++, while libstdc++ is present on the linker command line.

add_executable(RustMix main.rs)
target_link_libraries(
    RustMix
    rs_staticlib
    rs_cdylib
    rs_rlib
    c_static
    c_shared
    c_obj
    cpp_shared
)
