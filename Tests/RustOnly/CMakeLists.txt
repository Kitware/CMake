cmake_minimum_required(VERSION 4.2)

set(CMAKE_EXPERIMENTAL_RUST "3cc9b32c-47d3-4056-8953-d74e69fc0d6c")

project(RustOnly LANGUAGES Rust)

add_library(a STATIC a.rs)
add_library(b SHARED b.rs)
add_library(c OBJECT c.rs)

add_executable(RustOnly d.rs main.rs e.rs)
# Ensure that Rust_MAIN_CRATE_ROOT property works.
set_target_properties(RustOnly PROPERTIES Rust_MAIN_CRATE_ROOT main.rs)
# Emit an object file instead of an rlib.
set_source_files_properties(e.rs PROPERTIES Rust_EMIT "obj")
target_link_libraries(RustOnly a)
target_link_libraries(RustOnly b)
target_link_libraries(RustOnly c)
